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

const float GLSL_INFINITY = 1.0 / 0.0;
const float EPSILON = 0.001;
const uint MAX_ARR_SIZE = 64;

const float PI = 3.14159265358979323846;
const float PI_INV = 1.0 / PI;
const float PI_2_INV = 1.0 / (2.0 * PI);

const float RAY_TIME_MIN = 0.01;
const float RAY_TIME_MAX = 99999.9;
const uint RAY_TRACING_MAX_DEPTH = 100;

const float SHININESS = 32.0;
const float SPECULAR_STRENGTH = 0.5;
const float AMBIENT_STRENGTH = 0.1;

uint XORShift_RNG = 0;




