-- qr_thread.lua
local qrcode = require('lib.loveqrcode')
local running = true

-- Thread logic
while running do
    local data = love.thread.getChannel("totp_result_channel"):demand()  -- Wait for data

    if data then
        -- Calculate the TOTP
        qr1 = qrcode("https://zlgn.pl/view/dashboard?ploy=" .. id .. "&loyal=" .. totp)
        
        -- Send the result back
        love.thread.getChannel("qr_result_channel"):push(qr1)
		running = false
    end
end
