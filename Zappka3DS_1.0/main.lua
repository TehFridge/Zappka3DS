require("lib.text-draw")
--require("lib.bar128")
json = require("lib.json")
local qrcode = require('lib.loveqrcode')
local https = require("https")
local uuidgen = require("lib.uuid")
local ltn12 = require("ltn12")
local bit = require("bit")    
local sha1 = require("sha1")
local struct = require("lib.struct")
--local Barcode = require("lib.bar128") -- an awesome Code128 library made by Nawias (POLSKA GUROM)
local font = love.graphics.newFont("bold.ttf") -- Font lol
local reference = 0
local scrolltimerX = -100
local timer = 30
local SCREEN_WIDTH = 400 
local SCREEN_HEIGHT = 240
local exists = "dunno"
local state = ""
local elapsedTime = 0
local scrollDuration = 0.5  -- Total duration (in seconds) for the scrolling animation
local startY = 400          -- Initial Y position of the text
local endY = 210            -- Final Y position of the text
local startFade = 1          -- Initial Y position of the text
local endFade = 0.5            -- Final Y position of the text
local startbarY = 300          -- Initial Y position of the text
local endbarY = 20            -- Final Y position of the text
local isScrolling = false
local isFading = false
local currentY = startY
local banner = love.graphics.newImage("assets/banner.png")
local bottomdark = love.graphics.newImage("assets/frog.png")
local elapsedTimeFade = 0
local fadeDuration = 0.2
local stateChanged = false
local music = love.audio.newSource("bgm/bgm.ogg", "stream")
local sfx = love.audio.newSource("bgm/sfx.ogg", "static")
local sfx2 = love.audio.newSource("bgm/sfx2.ogg", "static")
local jsonread = true
name = "blank"
codeforinput = "blank"
loggedin = love.filesystem.exists("secret.hex.txt")
existsname = love.filesystem.exists("imie.txt")
love.graphics.setDefaultFilter("nearest")

if love._potion_version == nil then
	local nest = require("nest").init({ console = "3ds", scale = 1 })
	love._nest = true
    love._console_name = "3DS"
end

local function bytesToInt(bytes, index)
    local result = 0
    for i = index, index + 3 do
        result = result * 256 + bytes:byte(i)
    end
    return result
end

local function hmac_sha1(key, message)
    local blocksize = 64

    if #key > blocksize then
        key = sha1.binary(key)
    end

    key = key .. string.rep("\0", blocksize - #key)
    local o_key_pad = key:gsub('.', function(x) return string.char(bit.bxor(string.byte(x), 0x5c)) end)
    local i_key_pad = key:gsub('.', function(x) return string.char(bit.bxor(string.byte(x), 0x36)) end)

    return sha1.binary(o_key_pad .. sha1.binary(i_key_pad .. message))
end


function love.load()
	if existsname == false then -- Check whether the save file with the name exists or nah
        name = "3DS"
	else 
	    name =  love.filesystem.read("imie.txt")
	end
	
    if loggedin == false then -- Check whether the save file with the barcode exists or nah
        codeforinput = "101010101010"
		jsonread = false
		refresh_data("https://lorem-json.com/api/json", "", {}, "GET")
		if code == "0" then
			state = "wypierdalac_updateowac"
		else
			state = "login"
			jsonread = true
	    end
	  else 
	    codeforinput = love.filesystem.read("secret.hex.txt")
		id = love.filesystem.read("id.txt")
		calculatetotp()
		state = "main_strona"
    end
    --barcode = Barcode(codeforinput, 60, 3)
	music:setLooping(true)
    music:play()
end
function calculatetotp()
	local secret = codeforinput:gsub("%s", ""):lower()  -- wypierdol jakiś random shit (nwm ja to tylko portuje lol)
    local secretBytes = secret:gsub("(..)", function(hex)
        return string.char(tonumber(hex, 16))
    end)

    local ts = math.floor(os.time() / 30)
    local msg = struct.pack(">I8", ts)  -- Spakuj timestamp jako 64-bitowy big-endian integer

    local outputBytes = hmac_sha1(secretBytes, msg)

    local len = #outputBytes
    local lastByte = outputBytes:byte(len)
    local offset = bit.band(lastByte, 0xF) 
    local magicNumber = bytesToInt(outputBytes, offset + 1) % 1000000

    totp = string.format("%06d", magicNumber)
	qr1 = qrcode("https://zlgn.pl/view/dashboard?ploy=" .. id .. "&loyal=" .. totp)
end
function love.draw(screen)
    if screen == "bottom" then
        draw_bottom_screen()
    else
        draw_top_screen()
    end
end 

function reloadmenu()
    state = "main_strona"
end

function refresh_data(url, request, inheaders, metoda)
	love.filesystem.write("data.txt", request)
    -- Headers
    -- local myheaders = {
        -- ["user-agent"] = "Mozilla/5.0 (Windows NT 10.0; rv:129.0) Gecko/20100101 Firefox/129.0",
        -- ["accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/png,image/svg+xml,*/*;q=0.8",
        -- ["sec-fetch-user"] = "?1",
		-- ["sec-fetch-site"] = "none",
        -- ["sec-fetch-mode"] = "navigate",
        -- ["sec-fetch-dest"] = "document",
        -- ["accept-encoding"] = "gzip, deflate, br, zstd",
        -- ["accept-language"] = " pl,en-US;q=0.7,en;q=0.3",
		-- ["upgrade-insecure-requests"] = "1",
		-- ["te"] = "trailers",
		-- ["content-length"] = "0",
        -- ["priority"] = "u=0, i"
    -- }
    -- Response table to collect the response body
	local request_body = request --the json body
    response_body = {}
    -- Making the HTTP request
    code, body, headers = https.request(url, {data = request_body, method = metoda, headers = inheaders})
	love.filesystem.write("debug.txt", body)
	love.filesystem.write("status.txt", code)
	if jsonread == true then
		responded = json.decode(body)
	end
end




function draw_top_screen(dt)
    -- Calculate progress of the animation (from 0 to 1) for scrolling
    local progress = math.min(elapsedTime / scrollDuration, 1)
    local easedProgress = 1 - (1 - progress) ^ 2  -- Ease Out function: y = 1 - (1 - x)^2
    local currentY = startY + (endY - startY) * easedProgress
    local barY = startbarY + (endbarY - startbarY) * easedProgress

    -- Calculate progress of the fade animation (from 0 to 1) for fading
    local fadedprogress = math.min(elapsedTimeFade / fadeDuration, 1)
    local fadedProgres = 1 - (1 - fadedprogress) ^ 2  -- Ease Out function: y = 1 - (1 - x)^2
    local currentFade = startFade + (endFade - startFade) * fadedProgres

    -- Draw based on the current state
    if state == "main_strona" then
        -- Draw main screen elements with fade effect
		love.graphics.setColor(1, 1, 1, 1)
        love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		love.graphics.setColor(0.27,0.84,0.43,1)
        love.graphics.print('Cześć, ' .. name, font, 10, 10, 0, 3, 3)
    elseif state == "barcode" then
        -- Draw barcode screen elements with no fade effect
		love.graphics.setColor(1, 1, 1, currentFade)
		love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		love.graphics.setColor(1, 1, 1, 1)
		love.graphics.rectangle("fill", 5, barY - 10, SCREEN_WIDTH - 10, SCREEN_HEIGHT, 6, 6, 4)
		love.graphics.setColor(0, 0, 0, 1)
		love.graphics.rectangle("line", 5, barY - 10, SCREEN_WIDTH - 10, SCREEN_HEIGHT, 6, 6, 4)
		love.graphics.setColor(1, 1, 1, 1)
		love.graphics.setColor(0, 0, 0, 1)
		qr1:draw(95,barY,0,6.5)
		love.graphics.setColor(0.27,0.84,0.43,currentFade)
        --love.graphics.print('Cześć, ' .. name, font, 10, 10, 0, 3, 3)
		--TextDraw.DrawTextCentered(id, SCREEN_WIDTH/2, currentY - 25, {0,0,0,1}, font, 1.9)
		--TextDraw.DrawTextCentered("Zeskanuj swój Kod Kreskowy", SCREEN_WIDTH/2, currentY, {0.27,0.84,0.43,1}, font, 2.3)
	elseif state == "login" then
        -- Draw main screen elements with fade effect
		love.graphics.setColor(1, 1, 1, 1)
        love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		TextDraw.DrawTextCentered('Zaloguj się Numerem Telefonu', 200, 80, {0.27,0.84,0.43,1}, font, 2.7)
		TextDraw.DrawTextCentered('Wciśnij A', 200, 140, {0.27,0.84,0.43,1}, font, 3)
	elseif state == "smscode" then
		love.graphics.setColor(1, 1, 1, 1)
        love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		TextDraw.DrawTextCentered('By wprowadzić kod SMS', 200, 80, {0.27,0.84,0.43,1}, font, 2.7)
		TextDraw.DrawTextCentered('Wciśnij A', 200, 140, {0.27,0.84,0.43,1}, font, 3)
	elseif state == "loading" then
		love.graphics.setColor(1, 1, 1, 1)
        love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		TextDraw.DrawTextCentered(loadingtext, 200, 80, {0.27,0.84,0.43,1}, font, 2.7)
	elseif state == "wypierdalac_updateowac" then
		love.graphics.setColor(1, 1, 1, 1)
        love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		TextDraw.DrawTextCentered("Najprawdopodobniej nie masz certyfikatów SSL", 200, 80, {0.27,0.84,0.43,1}, font, 1.2)
		TextDraw.DrawTextCentered("Zupdate'uj Homebrew Launcher/hb-menu", 200, 100, {0.27,0.84,0.43,1}, font, 1.2)
    end
end

function draw_bottom_screen()
    SCREEN_WIDTH = 400
    SCREEN_HEIGHT = 240
    love.graphics.setColor(1, 1, 1, 1)
    love.graphics.draw(bottomdark, 0, 0)
    love.graphics.setColor(0, 0, 0, 1)
	if state == "main_strona" or state == "barcode" then
		love.graphics.print("A - Zobacz swój kod", font, 20, 10, 0, 1.2)
		love.graphics.print("X - Zmień swoją nazwe", font, 20, 40, 0, 1.2)
		love.graphics.setColor(1, 1, 1, 1)
	end
end

function love.gamepadpressed(joystick, button)
    if button == "a" then
        if state == "barcode" then
		    sfx2:play()
            state = "main_strona"
			isFading = false
        elseif state == "main_strona" then
		    sfx:play()
            state = "barcode"
			isScrolling = true
			isFading = true
            elapsedTime = 0  -- Reset elapsed time for scrolling animation
			elapsedTimeFade = 0
		elseif state == "login" then
			tel_login()
		elseif state == "smscode" then
			smskod()
			loadingtext = "Logowanie..."
			state = "loading"
        end
    end
    if button == "start" then
        love.event.quit()
    end
	if state == "barcode" then
		if button == "y" then
			changecode()
		end
	end
	if button == "x" then
        changename()
    end
	
end

function tel_login()
	if love._potion_version == nil then
		handle_authflow()
		numertel = "numer_telefonu"
		sendvercode("numer_telefonu")
		test()
		state = "smscode"
	else
		changes = "login"
		love.keyboard.setTextInput(true, {type = "numpad", hint = "Numer Telefonu."})
		love.keyboard.setTextInput(false)
		state = "smscode"
	end
end

function smskod()
	if love._potion_version == nil then
		sendbackvercode(love.filesystem.read("kurwa.txt"))
	else
		changes = "smscode"
		love.keyboard.setTextInput(true, {type = "numpad", hint = "Kod SMS."})
		love.keyboard.setTextInput(false)
	end
end
function test()
	local data = json.encode({idToken = boinaczejjebnie})
    refresh_data("https://www.googleapis.com/identitytoolkit/v3/relyingparty/getAccountInfo?key=AIzaSyDe2Fgxn_8HJ6NrtJtp69YqXwocutAoa9Q", data, {["Content-Type"] = "application/json", ["X-Android-Package"] = "pl.zabka.apb2c", ["X-Android-Cert"] = "FAB089D9E5B41002F29848FC8034A391EE177077", ["Accept-Language"] = "en-US", ["X-Client-Version"] = "Android/Fallback/X22003001/FirebaseCore-Android", ["X-Firebase-GMPID"] = "1:146100467293:android:0ec9b9022788ad32b7bfb4", ["X-Firebase-Client"] = "H4sIAAAAAAAAAKtWykhNLCpJSk0sKVayio7VUSpLLSrOzM9TslIyUqoFAFyivEQfAAAA", ["Content-Length"] = "894", ["User-Agent"] = "Dalvik/2.1.0 (Linux; U; Android 9; SM-A600FN Build/PPR1.180610.011)", ["Host"] = "www.googleapis.com", ["Connection"] = "Keep-Alive"}, "POST")
	--love.filesystem.write("tokendata.txt", data)
	--love.filesystem.write("tokendebug.txt", body)
end
function handle_authflow()
	local data = json.encode({clientType = "CLIENT_TYPE_ANDROID"})
    refresh_data("https://www.googleapis.com/identitytoolkit/v3/relyingparty/signupNewUser?key=AIzaSyDe2Fgxn_8HJ6NrtJtp69YqXwocutAoa9Q", data, {["content-type"] = "application/json"}, "POST")
	boinaczejjebnie = responded.idToken
	--love.filesystem.write("tokendata.txt", data)
	--love.filesystem.write("tokendebug.txt", body)
end
function sendvercode(nrtel)
	local data = json.encode({operationName = "SendVerificationCode", query = "mutation SendVerificationCode($input: SendVerificationCodeInput!) { sendVerificationCode(input: $input) { retryAfterSeconds } }",variables = {input = {phoneNumber = {countryCode = "48", nationalNumber = nrtel}}}})
	refresh_data("https://super-account.spapp.zabka.pl/", data, {["content-type"] = "application/json", ["authorization"] = responded.idToken}, "POST")
	--love.filesystem.write("data.txt", data)
	--love.filesystem.write("debug.txt", body)
end
function sendbackvercode(smscode)  --niby wyslij tylko kod sms, ale przy okazji weź mi cały auth flow zrób lmao
	local data = json.encode({operationName = "SignInWithPhone",variables = {input = {phoneNumber = {countryCode = "48", nationalNumber = numertel},verificationCode = smscode}}, query = "mutation SignInWithPhone($input: SignInInput!) { signIn(input: $input) { customToken } }"})
	refresh_data("https://super-account.spapp.zabka.pl/", data, {["content-type"] = "application/json", ["authorization"] = "Bearer " .. boinaczejjebnie, ["user-agent"] = "okhttp/4.12.0", ["x-apollo-operation-id"] = "a531998ec966db0951239efb91519560346cfecac77459fe3b85c5b786fa41de"	,["x-apollo-operation-name"] = "SignInWithPhone", ["accept"] = "multipart/mixed; deferSpec=20220824, application/json", ["content-length"] = "250"}, "POST")
	--love.filesystem.write("data.txt", data)
	--love.filesystem.write("debug.txt", body)
	local tokentemp = responded.data.signIn.customToken
	local data = json.encode({token = tokentemp, returnSecureToken = "true"})
	refresh_data("https://www.googleapis.com/identitytoolkit/v3/relyingparty/verifyCustomToken?key=AIzaSyDe2Fgxn_8HJ6NrtJtp69YqXwocutAoa9Q", data, {["content-type"] = "application/json"}, "POST")
	local tokentemp = responded.idToken
	local data = json.encode({idToken = tokentemp})
	refresh_data("https://www.googleapis.com/identitytoolkit/v3/relyingparty/getAccountInfo?key=AIzaSyDe2Fgxn_8HJ6NrtJtp69YqXwocutAoa9Q", data, {["content-type"] = "application/json"}, "POST")
	loadingtext = "Logowanie 45%..."
	uuidgen.seed()
	local data = json.encode({identityProviderToken = tokentemp, identityProvider = "OAUTH", apiKey = "b646c65e-a43d-4a61-9294-6c7c4385c762", uuid = uuidgen(), deviceId = "0432b18513e325a5"})
	-- refresh_data("https://zabka-snrs.zabka.pl/sauth/v3/auth/login/client/conditional", data, {["api-version"] = "4.4", ["application-id"] = "%C5%BCappka", ["user-agent"] = "Synerise Android SDK 5.9.0 pl.zabka.apb2c", ["accept"] = "application/json", ["mobile-info"] = "android;28;A600FNXXS5BTB2;9;SM-A600FN;samsung;5.9.0", ["content-type"] = "application/json; charset=UTF-8", ["content-length"] = "1140"}, "POST")
	-- loadingtext = "Logowanie 65%..."
	local data = ""
	refresh_data("https://qr-bff.spapp.zabka.pl/qr-code/secret", data, {["authorization"] = "Bearer " .. tokentemp, ["content-type"] = "application/json", ["accept"] = "application/json", ["app"] = "zappka-mobile", ["user-agent"] = "okhttp/4.12.0", ["content-length"] = "0"}, "GET")
	id = responded.userId
	love.filesystem.write("secret.hex.txt", responded.secrets.loyal)
	love.filesystem.write("id.txt", id)
	calculatetotp()
	state = "main_strona"
end
function changecode()
    changes = "bar"
	love.keyboard.setTextInput(true, {hint = "Write your name here."})
	love.keyboard.setTextInput(false)
end
	
function changename()
    changes = "name"
	love.keyboard.setTextInput(true, {hint = "Write your name here."})
	love.keyboard.setTextInput(false)
end
	
function love.textinput(text)
    if changes == "bar" then
	    codeforinput = text
		barcode:setCode(codeforinput)
		love.filesystem.write("kod.txt", text)
	elseif changes == "name" then
	    name = text
		love.filesystem.write("imie.txt", text)
	elseif changes == "login" then
		handle_authflow()
		numertel = text
		sendvercode(text)
	elseif changes == "smscode" then
		sendbackvercode(text)
	end
end
function love.update(dt)
    elapsedTime = elapsedTime + dt
	local time = love.timer.getTime()  -- Get current time in seconds
	if scrolltimerX <= 550 then
	   scrolltimerX = scrolltimerX + 0.2
	else
	   scrolltimerX = -150
	end
	-- Update the scrolling animation if isScrolling is true
    if isScrolling == true then
        -- Calculate progress of the animation (from 0 to 1)
        local progress = math.min(elapsedTime / scrollDuration, 1)
	    local fadedprogress = math.min(elapsedTime / fadeDuration, 1)

        -- Apply ease-out function to the progress (for smoother scrolling)
        local easedProgress = 1 - (1 - progress) ^ 2  -- Ease Out function: y = 1 - (1 - x)^2

        -- Interpolate Y position based on eased progress
        local currentY = startY + (endY - startY) * easedProgress
    	local barY = startbarY + (endbarY - startbarY) * easedProgress
    end
	
	    local fadedprogress = math.min(elapsedTime / fadeDuration, 1)
	    local fadedProgres = 1 - (1 - fadedprogress) ^ 2  -- Ease Out function: y = 1 - (1 - x)^2
		local currentFade = startFade + (endFade - startFade) * fadedProgres
		
	    
    elapsedTimeFade = elapsedTimeFade + dt

    -- Check if the transition animation (scrolling) has completed
    if elapsedTime >= scrollDuration then
        isScrolling = false  -- Animation complete
    end

    -- Check if the fade animation has completed
    if elapsedTimeFade >= fadeDuration then
        isFading = false  -- Fade animation complete
    end

    -- If transitioning to or from a state, reset fade animation timer
    if stateChanged then
        elapsedTimeFade = 0
        isFading = true
        stateChanged = false
    end
	timer = timer - dt
	if state == "barcode" then
		if timer <= 0 then
			-- Restart the timer
			timer = 30
			calculatetotp()
		end
	end
    love.graphics.origin()  
end