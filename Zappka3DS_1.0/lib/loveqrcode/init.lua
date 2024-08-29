local LOVE_QRCODE = {
    _VERSION     = "0.0.1",
    _DESCRIPTION =
    "QR Code rendering library for LÖVE, based on Luaqrcode (https://github.com/speedata/luaqrcode)",
    _URL         = "https://github.com/Nawias/love-qrcode",
    _LICENSE     = [[
        Copyright 2024 Michał Wójcik

        Permission is hereby granted, free of charge, to any person obtaining a
        copy of this software and associated documentation files (the
        "Software"), to deal in the Software without restriction, including
        without limitation the rights to use, copy, modify, merge, publish,
        distribute, sublicense, and/or sell copies of the Software, and to
        permit persons to whom the Software is furnished to do so, subject to
        the following conditions:

        The above copyright notice and this permission notice shall be included
        in all copies or substantial portions of the Software.

        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
        OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
        MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
        IN NO EVENTs SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
        CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
        TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
        SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
        ]],
}
local libpath = (...):gsub('%.init$', '')
local qrencode = require(libpath .. '.qrencode')

---@class QRCode
---@field private text string Text to be encoded
---@field private size number Size of the qrcode (in pixels)
---@field private buffer love.Image buffer that will get drawn
---@field private __index QRCode
---@overload fun(text:string):QRCode
local QRCode = {}
QRCode.__index = QRCode

--#region Private methods

---Encodes the QR code
---@param self QRCode
local function encode(self)
    local ok, result = qrencode.qrcode(self.text, 1)
    assert(ok, result)
    self.size = #result
    local buffer = love.image.newImageData(self.size, self.size)
    buffer:mapPixel(function (y,x)
        local r = result[y+1][x+1] > 0 and 1 or 0
        return r,r,r,r 
    end)
    if self.buffer then self.buffer:release() end
    self.buffer = love.graphics.newImage(buffer)
    buffer:release()
    
end
--#endregion

--#region Public methods

---QR Code Constructor
---@param self QR Code
---@param text string Text to be encoded
local function newQRCode(self, text)
    assert(('string') == type(text) and string.len(text) > 0, "QRCode: invalid text")
    local object = {
        text = text,
        buffer = nil,
        size = nil
    }
    encode(object)
    object = setmetatable(object, QRCode)
    return object
end

---@overload fun(self:QRCode, transform: love.Transform)
---@overload fun(self:QRCode, quad: love.Quad, transform: love.Transform)
---Draws the QR code on the screen
---@param x number? The position to draw the object (x-axis)
---@param y number? The position to draw the object (y-axis)
---@param r number? Orientation (radians)
---@param sx number? Scale factor (x-axis)
---@param sy number? Scale factor (y-axis)
---@param ox number? Origin offset (x-axis)
---@param oy number? Origin offset (y-axis)
---@param kx number? Shearing factor (x-axis)
---@param ky number? Shearing factor (y-axis)
function QRCode:draw(...)
    love.graphics.draw(self.buffer, ...)
end

---Gets the current text value
---@return string
function QRCode:getText()
    return self.text
end

---Changes the value of the QR Code
---@param text string The text you want to encode
function QRCode:setText(text)
    self.text = text
    encode(self)
end

---Gets the buffer size in pixels
---@return number
function QRCode:getSize()
    return self.size
end

--#region

setmetatable(QRCode, { __call = newQRCode })
return QRCode