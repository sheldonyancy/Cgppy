/**
 * MIT License
 *
 * Copyright (c) 2024 Sheldon Yancy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

import Foundation
import Cocoa
import SwiftUI
import MetalKit

public struct YsUniformModel {
    var model_matrix: matrix_float4x4
}

public struct YsUniformCamera {
    var view_matrix: matrix_float4x4
    var projection_matrix: matrix_float4x4
}

public class YMetalViewInteractor: NSObject, ObservableObject {
    let metal_view = MTKView()
    private let device: MTLDevice
    private let command_queue: MTLCommandQueue
    private let pipeline_state: MTLRenderPipelineState
    private var uniform_model: YsUniformModel = YsUniformModel(model_matrix:matrix_identity_float4x4)
    private var uniform_camera: YsUniformCamera = YsUniformCamera(view_matrix:matrix_identity_float4x4, projection_matrix:matrix_identity_float4x4)
    private var vertex_count: Int = 0
    private var vertex_position_buffer: MTLBuffer!
    private var vertex_normal_buffer: MTLBuffer!

    public override init() {
        guard let device = MTLCreateSystemDefaultDevice(),
              let command_queue = device.makeCommandQueue()
              else { fatalError() }

        self.device = device
        self.command_queue = command_queue

        self.metal_view.device = device
        self.metal_view.clearColor = MTLClearColor(red: 0.0, green: 0.0, blue: 0.0, alpha: 1.0)

        let metallib_file = "./Assets/ShaderMetal/shader.metallib"
        let metallib_url = URL(fileURLWithPath: metallib_file)

        do {
            let metallib_data = try Data(contentsOf: metallib_url)
            let dispatch_data = metallib_data.withUnsafeBytes {
                (bytes: UnsafeRawBufferPointer) -> DispatchData in return DispatchData(bytes: bytes)
            }

            let default_library = try device.makeLibrary(data: dispatch_data)

            guard let vertex_function = default_library.makeFunction(name: "vertex_main"),
                  let fragment_function = default_library.makeFunction(name: "fragment_main")
            else {
                fatalError("Failed to load the shaders")
            }

            let pipeline_state_descriptor = MTLRenderPipelineDescriptor()
            pipeline_state_descriptor.label = "Polyhedron Pipeline"
            pipeline_state_descriptor.vertexFunction = vertex_function
            pipeline_state_descriptor.fragmentFunction = fragment_function
            pipeline_state_descriptor.colorAttachments[0].pixelFormat = self.metal_view.colorPixelFormat

            self.pipeline_state = {
                do {
                    return try device.makeRenderPipelineState(descriptor: pipeline_state_descriptor)
                } catch {
                    fatalError()
                }
            }()

            super.init()

            self.metal_view.delegate = self
        } catch {
            fatalError("Failed to load metallib file: \(error)")
        }
    }

    public func setVertexPosition(_ data: UnsafePointer<Float>, _ count: Int) {
        self.vertex_count = count / 3

        var tmp: [vector_float3] = []

        for i in 0..<self.vertex_count {
            let x = data[3 * i]
            let y = data[3 * i + 1]
            let z = data[3 * i + 2]
            tmp.append(vector_float3(x, y, z))
        }
        self.vertex_position_buffer = self.device.makeBuffer(bytes: tmp,
                                                             length: MemoryLayout<vector_float3>.size * tmp.count,
                                                             options: .storageModeShared)
    }
    public func setVertexNormal(_ data: UnsafePointer<Float>, _ count: Int) {
        let tmp_count = count / 3
        var tmp: [vector_float3] = []

            for i in 0..<tmp_count {
                let x = data[3 * i]
                let y = data[3 * i + 1]
                let z = data[3 * i + 2]
                tmp.append(vector_float3(x, y, z))
            }
        self.vertex_normal_buffer = self.device.makeBuffer(bytes: tmp,
                                                           length: MemoryLayout<vector_float3>.size * tmp.count,
                                                           options: .storageModeShared)
    }
    public func setModelMatrix(_ matrix_pointer: UnsafePointer<Float>) {
        self.uniform_model.model_matrix = matrix_float4x4(columns: (
            SIMD4<Float>(matrix_pointer[0],  matrix_pointer[1],  matrix_pointer[2],  matrix_pointer[3]),
            SIMD4<Float>(matrix_pointer[4],  matrix_pointer[5],  matrix_pointer[6],  matrix_pointer[7]),
            SIMD4<Float>(matrix_pointer[8],  matrix_pointer[9],  matrix_pointer[10], matrix_pointer[11]),
            SIMD4<Float>(matrix_pointer[12], matrix_pointer[13], matrix_pointer[14], matrix_pointer[15])))
    }
    public func setViewMatrix(_ matrix_pointer: UnsafePointer<Float>) {
        self.uniform_camera.view_matrix = matrix_float4x4(columns: (
            SIMD4<Float>(matrix_pointer[0],  matrix_pointer[1],  matrix_pointer[2],  matrix_pointer[3]),
            SIMD4<Float>(matrix_pointer[4],  matrix_pointer[5],  matrix_pointer[6],  matrix_pointer[7]),
            SIMD4<Float>(matrix_pointer[8],  matrix_pointer[9],  matrix_pointer[10], matrix_pointer[11]),
            SIMD4<Float>(matrix_pointer[12], matrix_pointer[13], matrix_pointer[14], matrix_pointer[15])))
    }
    public func setProjectionMatrix(_ matrix_pointer: UnsafePointer<Float>) {
        self.uniform_camera.projection_matrix = matrix_float4x4(columns: (
            SIMD4<Float>(matrix_pointer[0],  matrix_pointer[1],  matrix_pointer[2],  matrix_pointer[3]),
            SIMD4<Float>(matrix_pointer[4],  matrix_pointer[5],  matrix_pointer[6],  matrix_pointer[7]),
            SIMD4<Float>(matrix_pointer[8],  matrix_pointer[9],  matrix_pointer[10], matrix_pointer[11]),
            SIMD4<Float>(matrix_pointer[12], matrix_pointer[13], matrix_pointer[14], matrix_pointer[15])))
    }
}

extension YMetalViewInteractor: MTKViewDelegate {
    public func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {

        }

    public func draw(in view: MTKView) {
        guard vertex_position_buffer != nil
              else { return }

        guard let drawable = view.currentDrawable,
              let command_buffer = command_queue.makeCommandBuffer(),
              let render_pass_descriptor = view.currentRenderPassDescriptor,
              let render_encoder = command_buffer.makeRenderCommandEncoder(descriptor: render_pass_descriptor)
              else { fatalError() }

        render_encoder.setRenderPipelineState(self.pipeline_state)

        render_encoder.setVertexBuffer(self.vertex_position_buffer, offset: 0, index: 0)
        render_encoder.setVertexBuffer(self.vertex_normal_buffer, offset: 0, index: 1)
        render_encoder.setVertexBytes(&self.uniform_model, length: MemoryLayout<YsUniformModel>.size, index: 2)
        render_encoder.setVertexBytes(&self.uniform_camera, length: MemoryLayout<YsUniformCamera>.size, index: 3)

        render_encoder.drawPrimitives(type: .triangle, vertexStart: 0, vertexCount: self.vertex_count)
        render_encoder.endEncoding()
        command_buffer.present(drawable)
        command_buffer.commit()
    }
}

struct YsMetalViewRepresentable: NSViewRepresentable {
    let metal_view: MTKView
    func makeNSView(context: Context) -> MTKView { self.metal_view }
    func updateNSView(_ nsView: MTKView, context: Context) {}
    typealias NSViewType = MTKView
}

public struct YsContentView: View {
    @ObservedObject var view_interactor: YMetalViewInteractor

    public init(view_interactor: YMetalViewInteractor = YMetalViewInteractor()) {
        self._view_interactor = ObservedObject(wrappedValue: view_interactor)
    }

    public var body: some View {
        VStack {
            YsMetalViewRepresentable(metal_view: self.view_interactor.metal_view)
        }
    }

    public func setVertexPosition(_ data: UnsafePointer<Float>, _ count: Int) {
        self.view_interactor.setVertexPosition(data, count)
    }

    public func setVertexNormal(_ data: UnsafePointer<Float>, _ count: Int) {
        self.view_interactor.setVertexNormal(data, count)
    }
    
    public func setModelMatrix(_ matrix_pointer: UnsafePointer<Float>) {
        self.view_interactor.setModelMatrix(matrix_pointer)
    }

    public func setProjectionMatrix(_ matrix_pointer: UnsafePointer<Float>) {
        self.view_interactor.setProjectionMatrix(matrix_pointer)
    }
}
