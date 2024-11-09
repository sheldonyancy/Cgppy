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


@objc (YExternalInterface)
public class YExternalInterface: NSObject {
    public var content_view: YsContentView
    public var window: NSWindow

    @objc public override init() {
        self.content_view = YsContentView()
        self.window = NSWindow(
                    contentRect: NSRect(x: 0, y: 0, width: 1920, height: 1080),
                    styleMask: [.titled, .closable, .miniaturizable, .resizable, .fullSizeContentView],
                    backing: .buffered, defer: false)
        self.window.center()
        self.window.setFrameAutosaveName("Metal")
        self.window.contentView = NSHostingView(rootView: self.content_view)
        //self.window.makeKeyAndOrderFront(nil)
        self.window.orderOut(nil)
    }

    @objc public func setVertexPosition(_ data: UnsafePointer<Float>, _ count: Int) {
        self.content_view.setVertexPosition(data, count)
    }

    @objc public func setVertexNormal(_ data: UnsafePointer<Float>, _ count: Int) {
        self.content_view.setVertexNormal(data, count)
    }

    @objc public func setModelMatrix(_ matrix_pointer: UnsafePointer<Float>) {
        self.content_view.setModelMatrix(matrix_pointer)
    }

    @objc public func setProjectionMatrix(_ matrix_pointer: UnsafePointer<Float>) {
        self.content_view.setProjectionMatrix(matrix_pointer)
    }
}

