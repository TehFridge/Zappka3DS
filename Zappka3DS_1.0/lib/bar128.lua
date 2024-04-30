local BAR128 = {
    _VERSION     = "0.9.1",
    _DESCRIPTION =
    "Barcode rendering library for LÖVE, based on Zanstra's JS implementation (https://zanstra.com/my/Barcode/code128.js)",
    _URL         = "https://github.com/Nawias/bar128-love",
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
---@class Barcode
---@field private code string|number Text to be encoded
---@field private barcodeType 'A'|'B'|'C' Code 128 character set
---@field private barHeight number Height of the drawn barcode
---@field private barWidth number width of the individual bars in pixels (default: `1`)
---@field private cachedBarcode number[] Parsed barcode ready for rendering
---@field private __index Barcode
local Barcode = {}
Barcode.__index = Barcode

local BARS = {
    [0] = 212222,
    222122,
    222221,
    121223,
    121322,
    131222,
    122213,
    122312,
    132212,
    221213,
    221312,
    231212,
    112232,
    122132,
    122231,
    113222,
    123122,
    123221,
    223211,
    221132,
    221231,
    213212,
    223112,
    312131,
    311222,
    321122,
    321221,
    312212,
    322112,
    322211,
    212123,
    212321,
    232121,
    111323,
    131123,
    131321,
    112313,
    132113,
    132311,
    211313,
    231113,
    231311,
    112133,
    112331,
    132131,
    113123,
    113321,
    133121,
    313121,
    211331,
    231131,
    213113,
    213311,
    213131,
    311123,
    311321,
    331121,
    312113,
    312311,
    332111,
    314111,
    221411,
    431111,
    111224,
    111422,
    121124,
    121421,
    141122,
    141221,
    112214,
    112412,
    122114,
    122411,
    142112,
    142211,
    241211,
    221114,
    413111,
    241112,
    134111,
    111242,
    121142,
    121241,
    114212,
    124112,
    124211,
    411212,
    421112,
    421211,
    212141,
    214121,
    412121,
    111143,
    111341,
    131141,
    114113,
    114311,
    411113,
    411311,
    113141,
    114131,
    311141,
    411131,
    211412,
    211214,
    211232,
    23311120
}
local START_CODE = {
    A = 103,
    B = 104,
    C = 105
}
local STOP = 106

--#region Private functions
local function isBarcodeType(barcodeType)
    return barcodeType == 'A' or barcodeType == 'B' or barcodeType == 'C'
end

local fromType = {
    A = function(charCode)
        if charCode >= 0 and charCode < 32 then return charCode + 64 end
        if charCode >= 32 and charCode < 96 then return charCode - 32 end
        return charCode
    end,
    B = function(charCode)
        if charCode >= 32 and charCode < 128 then return charCode - 32 end
        return charCode
    end,
    C = function(charCode)
        return charCode
    end
}

---Parses code into bars
---@param code string|number Alphanumerical code
---@param barcodeType 'A'|'B'|'C'
---@return number[] bars Code parsed into bars
local function parseBarcode(code, barcodeType)
    if (barcodeType == 'C' and string.len(code) % 2 == 1) then
        -- Append a leading 0 for digit parity
        code = '0' .. code
    end
    local bars = {}
    local checksum = 0
    local function add(barIndex)
        local nrCode = BARS[barIndex]
        local barsLen = #bars
        checksum = (barsLen == 0) and barIndex or checksum + barIndex * barsLen
        table.insert(bars, nrCode or ('UNDEFINED: ' .. barIndex .. '->' .. nrCode))
    end
    add(START_CODE[barcodeType])
    local i = 1
    repeat
        local barCode
        if barcodeType == 'C' then
            barCode = tonumber(string.sub(code, i, i + 1))
            i = i + 1
        else
            barCode = string.byte(code, i)
        end
        local converted = fromType[barcodeType](barCode)
        if ('number' ~= type(converted) or converted < 0 or converted > 106) then
            error("Unrecognized character (" .. barCode .. ") at position " .. i .. " in code '" .. code .. "'.")
        end
        add(converted)

        i = i + 1
    until i > string.len(code)

    table.insert(bars, BARS[checksum % 103])
    table.insert(bars, BARS[STOP])
    return bars
end

---Detects barcode charset based on characters used
---@param code string|number
---@return 'A'|'B'|'C'
local function detectBarcodeType(code)
    if code:match("^%d+$") then return 'C' end
    if code:match("%l+") then return 'B' end
    return 'A'
end

--#endregion

--#region Public methods

---Barcode Object Constructor
---@param self Barcode
---@param code string|number code you want rendered as barcode
---@param barHeight number? height of the barcode image (default: 20)
---@param barWidth number? width of one bar (default: 1)
---@param barcodeType 'A'|'B'|'C'|nil
---@return Barcode barcode
local function newBarcode(self, code, barHeight, barWidth, barcodeType)
    assert(('string'==type(code) or 'number'==type(code)) and string.len(code)>1,"Barcode: invalid code")
    assert(isBarcodeType(barcodeType) or barcodeType == nil, "Barcode: invalid barcode type")
    assert(barHeight>0,"Barcode: bar height cannot be 0 or less")
    assert(barWidth>0,"Barcode: bar width cannot be 0 or less")
    local object = {
        code = code,
        barHeight = barHeight or 20,
        barWidth = barWidth or 1,
        barcodeType = barcodeType or detectBarcodeType(code)
    }
    object.cachedBarcode = parseBarcode(code, object.barcodeType)
    object = setmetatable(object, Barcode)
    return object
end


---Draws the barcode
---@param text 'notext'|'text' Indicates whether the text representation should be drawn underneath the barcode.
---@param x number X coordinate
---@param y number Y coordinate
function Barcode:draw(text,x, y)
    -- Account for the quiet zone
    local cursor = x + self.barWidth*10

    for _, glyph in pairs(self.cachedBarcode) do
        local isDrawing = true
        for i = 1, string.len(glyph), 1 do
            local width = tonumber(string.sub(glyph, i, i)) * self.barWidth
            if isDrawing then love.graphics.rectangle("fill", cursor, y, width, self.barHeight) end
            cursor = cursor + width
            isDrawing = not isDrawing
        end
    end
end

--#region Getters, Setters

function Barcode:getCode()
    return self.code
end

---Changes barcode's value
---
---`⚠️ Warning`: changing the value causes the barcode to be re-parsed internally. Don't use this too often.
---@param code string|number New barcode value
---@param barcodeType 'A'|'B'|'C'|nil (Optional) Force charset
function Barcode:setCode(code, barcodeType)
    assert(('string'==type(code) or 'number'==type(code)) and string.len(code)>1,"Barcode: invalid code")
    assert(isBarcodeType(barcodeType) or barcodeType == nil, "Barcode: invalid barcode type")
    self.code = code
    self.barcodeType = barcodeType or detectBarcodeType(code)
    self.cachedBarcode = parseBarcode(code, self.barcodeType)
end

---Get the current bar width
---@return number barWidth Bar width
function Barcode:getBarWidth()
    return self.barWidth
end

---Changes the base width of bars
---@param width number
function Barcode:setBarWidth(width)
    assert(width>0,"Barcode: bar width cannot be 0 or less")
    self.barWidth = width
end

---Get the current barcode height
---@return number bar height Barcode height
function Barcode:getBarHeight()
    return self.barHeight
end

---Changes the base width of bars
---@param height number
function Barcode:setBarHeight(height)
    assert(height>0,"Barcode: bar height cannot be 0 or less")
    self.barHeight = height
end
--#endregion

--#endregion

setmetatable(Barcode, { __call = newBarcode })

return Barcode
