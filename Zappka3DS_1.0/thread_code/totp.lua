-- totp_thread.lua
local bit = require("bit")  -- Make sure you require bit manipulation
local sha1 = require("sha1")  -- Assuming sha1 library is available
local qrcode = require('lib.loveqrcode')
local running = true
-- Helper functions for TOTP calculation
local function c(arr, index)
    local result = 0
    for i = index, index + 4 do
        result = bit.bor(bit.lshift(result, 8), bit.band(arr:byte(i), 0xFF))
    end
    return result
end

local function calculateTOTP(code,id)
    local javaIntMax = 2147483647
    local secretHex = dawajto
    local secret = (secretHex:gsub('..', function(hex)
        return string.char(tonumber(hex, 16))
    end))

    local czas = os.time()
    local ts = math.floor(czas / 30)
    local msg = struct.pack(">L8", ts)

    local outputBytes = sha1.hmac_binary(secret, msg)
    local magicNumber = bit.band(c(outputBytes, bit.band(outputBytes:byte(#outputBytes), 15)), 2147483647) % 1000000

    local totp = string.format("%06d", magicNumber)
	local qr1 = qrcode("https://zlgn.pl/view/dashboard?ploy=" .. id .. "&loyal=" .. totp)
	print("returned")
	return qr1
end

-- Thread logic
while running do
    local data = love.thread.getChannel("totp_channel"):demand()  -- Wait for data
	print("Received data:", data.dawajto, data.id)

    if data.codeforinput then
        -- Calculate the TOTP
        local qr1 = calculateTOTP(data.dawajto, data.id)
        
        -- Send the result back
        love.thread.getChannel("totp_result_channel"):push(qr1)
		running = false
    end
end
