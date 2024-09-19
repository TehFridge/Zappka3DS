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
--local ffi = require("ffi")
--local Barcode = require("lib.bar128") -- an awesome Code128 library made by Nawias (POLSKA GUROM)
local font = love.graphics.newFont("bold.ttf") -- Font lol
local reference = 0
local scrolltimerX = -100
local option_sel = 1
local promka_sel = 1
local qrcal = "false"
local imageload = "true"
local promka_sel2 = 1
local SCREEN_WIDTH = 400 
local SCREEN_HEIGHT = 240
local image = false
local timer = 30
local exists = "dunno"
local intranet = "false"
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
gui_design_mode = false

if love._potion_version == nil then
	local nest = require("nest").init({ console = "3ds", scale = 1 })
	love._nest = true
    love._console_name = "3DS"
end




function love.load()
    -- Get the current time
	optiontable = {imageload, qrcal, intranet}
	generated_once = false
	if existsname == false then -- Check whether the save file with the name exists or nah
        name = "3DS"
	else 
	    name =  love.filesystem.read("imie.txt")
	end
	
    if loggedin == false then -- Check whether the save file with the barcode exists or nah
        codeforinput = "101010101010"
		jsonread = false
		updatezappsy()
		if code == "0" then
			state = "wypierdalac_updateowac"
		else
			state = "login"
			jsonread = true
	    end
	  else 
	    codeforinput = love.filesystem.read("secret.hex.txt")
		id = love.filesystem.read("id.txt")
		authtoken = love.filesystem.read("token.txt")
		jsonread = false
		updatezappsy() --taki test by zobaczyć czy masz neta
		if not string.find(body, "uuid") then
			optiontable[3] = "true"
		else
			optiontable[3] = "false"
			jsonread = true
			updatezappsy()
			zappsy = responded.content.points
			if updatetime_withserver() < 1727128800 then
				promki_table = {"Prosto_z_Pieca", "kat_lody", "kat_napoje", "kat_wiekszyglod", "kat_piwo", "SSF_kupony"}
				promki_nametable = {"Streetfood", "Lody", "Napoje", "Głodny?", "Może Piwko?", "Festiwal kuponów!"}
				SSF = true
				print("SSF true")
			else
				promki_table = {"Prosto_z_Pieca", "kat_lody", "kat_napoje", "kat_wiekszyglod", "kat_piwo"}
				promki_nametable = {"Streetfood", "Lody", "Napoje", "Głodny?", "Może Piwko?"}
				SSF = false
				print("SSF false")
			end
			refresh_data("https://zabka-snrs.zabka.pl/schema-service/v2/documents/specialoffersettings/generate", data, {["api-version"] = "4.4", ["application-id"] = "%C5%BCappka", ["user-agent"] = "Synerise Android SDK 5.9.0 pl.zabka.apb2c", ["accept"] = "application/json", ["mobile-info"] = "horizon;28;AW700000000;9;CTR-001;nintendo;5.9.0", ["content-type"] = "application/json; charset=UTF-8", ["authorization"] = authtoken}, "GET")
			if responded.content.alcohol_eligibility == "1" then
				piweczko = true
			end
	    end
		state = "main_strona"
    end
    --barcode = Barcode(codeforinput, 60, 3)
	music:setLooping(true)
    music:play()
end
local function isLeapYear(year)
    return (year % 4 == 0 and year % 100 ~= 0) or (year % 400 == 0)
end

local function daysInMonth(month, year)
    local days = {31, isLeapYear(year) and 29 or 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    return days[month]
end

local function isoToUnix(isoDate)
    local pattern = "(%d+)-(%d+)-(%d+)T(%d+):(%d+):(%d+)%.(%d+)Z"
    local year, month, day, hour, min, sec, ms = isoDate:match(pattern)

    year, month, day = tonumber(year), tonumber(month), tonumber(day)
    hour, min, sec = tonumber(hour), tonumber(min), tonumber(sec)

    -- Days calculation since the Unix epoch (1970-01-01)
    local days = 0
    for y = 1970, year - 1 do
        days = days + (isLeapYear(y) and 366 or 365)
    end

    for m = 1, month - 1 do
        days = days + daysInMonth(m, year)
    end

    days = days + (day - 1)

    -- Convert to total seconds
    local totalSeconds = days * 86400 + hour * 3600 + min * 60 + sec

    return totalSeconds
end
function calculatetotp() --NAPRAWIŁEM KURWA
	local javaIntMax = 2147483647

	local function c(arr, index)
		local result = 0
		for i = index, index + 4 do
			result = bit.bor(bit.lshift(result, 8), bit.band(arr:byte(i), 0xFF))
		end
		return result
	end
	local secretHex = codeforinput
	local secret = (secretHex:gsub('..', function(hex)
        return string.char(tonumber(hex, 16))
    end))
	if optiontable[3] == "true" then
		if optiontable[2] == "false" then
			czas = os.time()
			print("intranet: " .. czas)
		else
			czas = alt_kalibracja()
		end
	else
		czas = updatetime_withserver()
		print("internet: " .. czas)
	end
	if love._console == "3ds" then
		ts = math.floor(czas / 30)
	else 
		ts = math.floor(czas / 30)
	end
	print(ts)
    local msg = struct.pack(">L8", ts)

    local outputBytes = sha1.hmac_binary(secret, msg)

    local magicNumber = bit.band(c(outputBytes, bit.band(outputBytes:byte(#outputBytes), 15)), 2147483647) % 1000000

    totp = string.format("%06d", magicNumber)
	print(totp)
	print("https://zlgn.pl/view/dashboard?ploy=" .. id .. "&loyal=" .. totp)
	qr1 = qrcode("https://zlgn.pl/view/dashboard?ploy=" .. id .. "&loyal=" .. totp)
	generated_once = true
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
	--print(request)
	--love.filesystem.write("data.txt", request)
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
	if image == false then
		code, body, headers = https.request(url, {data = request_body, method = metoda, headers = inheaders})
	else
		code, imagebody, headers = https.request(url, {data = request_body, method = metoda, headers = inheaders})
	end
	--print(body)
	--print(code)
	if jsonread == true then
		responded = json.decode(body)
	end
end




function draw_top_screen(dt)
    if isScrolling == true then
        -- Calculate progress of the animation (from 0 to 1)
        progress = math.min(elapsedTime / scrollDuration, 1)
		fadedprogress = math.min(elapsedTimeFade / fadeDuration, 1)
		fadedProgres = 1 - (1 - fadedprogress) ^ 2  -- Ease Out function: y = 1 - (1 - x)^2
		currentFade = startFade + (endFade - startFade) * fadedProgres

        -- Apply ease-out function to the progress (for smoother scrolling)
        easedProgress = 1 - (1 - progress) ^ 2  -- Ease Out function: y = 1 - (1 - x)^2

        -- Interpolate Y position based on eased progress
        currentY = startY + (endY - startY) * easedProgress
    	barY = startbarY + (endbarY - startbarY) * easedProgress
    end

    -- Draw based on the current state
    if state == "main_strona" then
        -- Draw main screen elements with fade effect
		love.graphics.setColor(1, 1, 1, 1)
        love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		love.graphics.setColor(0.27,0.84,0.43,1)
        love.graphics.print('Cześć, ' .. name, font, 10, 10, 0, 3, 3)
		if optiontable[3] == "false" then
			love.graphics.print('Ilość Żappsów: ' .. zappsy, font, 10, 45, 0, 2, 2)
		else
			love.graphics.print('Ilość Żappsów: Chwilowy Brak Żappsów', font, 10, 45, 0, 2, 2)
			love.graphics.setColor(0.77,0.04,0.03,1)
			love.graphics.print('! Brak Dostępu do Internetu !', font, 0, 195, 0, 1.2, 1.2)
			love.graphics.print('Kody QR mogą się źle generować', font, 0, 210, 0, 1.2, 1.2)
			love.graphics.print('(nwm czas w 3dsach jakiś zjebany jest)', font, 0, 225, 0, 1.2, 1.2)
		end
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
		if generated_once == true then
			qr1:draw(95,barY,0,6.5)
		else
			love.graphics.printf("Wygeneruj Kod QR", font, 5, barY + 90, 250, "center", 0, 1.55, 1.55)
		end
		love.graphics.setColor(0.27,0.84,0.43,currentFade)
		--love.graphics.print(totp, font, 20, 10, 0, 1.2)
		--love.graphics.print(ts, font, 20, 30, 0, 1.2)
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
    elseif state == "promki" then
        -- Draw barcode screen elements with no fade effect
		love.graphics.setColor(1, 1, 1, 1)
		love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		love.graphics.setColor(0.27,0.84,0.43,1)
		love.graphics.print("->", font, -barY + 20, (promka_sel2 * 25 ) + 18, 0, 2)
		TextDraw.DrawTextCentered(promki_nametable[promka_sel], SCREEN_WIDTH/2, 25, {0.27,0.84,0.43, 1}, font, 3.2)
		TextDraw.DrawTextCentered(responded[1].content.name, SCREEN_WIDTH/2, 60, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered(responded[2].content.name, SCREEN_WIDTH/2, 85, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered(responded[3].content.name, SCREEN_WIDTH/2, 110, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered(responded[4].content.name, SCREEN_WIDTH/2, 135, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered(responded[5].content.name, SCREEN_WIDTH/2, 160, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered(responded[6].content.name, SCREEN_WIDTH/2, 185, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered(responded[7].content.name, SCREEN_WIDTH/2, 210, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered(responded[8].content.name, SCREEN_WIDTH/2, 235, {0.27,0.84,0.43, 1}, font, 1.9)
	elseif state == "promki_sel" then
        -- Draw barcode screen elements with no fade effect
		love.graphics.setColor(1, 1, 1, 1)
		love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		love.graphics.setColor(0.27,0.84,0.43,1)
		love.graphics.print("->", font, -barY + 50, (promka_sel * 26 ) + 10, 0, 3)
		TextDraw.DrawTextCentered("Kupony:", SCREEN_WIDTH/2, -barY + 45, {0.27,0.84,0.43, 1}, font, 3.2)
		TextDraw.DrawTextCentered(promki_nametable[1], SCREEN_WIDTH/2 + barY - 20, 60, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered(promki_nametable[2], SCREEN_WIDTH/2 + barY - 20, 85, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered(promki_nametable[3], SCREEN_WIDTH/2 + barY - 20, 110, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered(promki_nametable[4], SCREEN_WIDTH/2 + barY - 20, 135, {0.27,0.84,0.43, 1}, font, 1.9)
		if piweczko == true then
			TextDraw.DrawTextCentered(promki_nametable[5], SCREEN_WIDTH/2 + barY - 20, 160, {0.27,0.84,0.43, 1}, font, 1.9)
		end
        --love.graphics.print('Cześć, ' .. name, font, 10, 10, 0, 3, 3)
		--TextDraw.DrawTextCentered(id, SCREEN_WIDTH/2, currentY - 25, {0,0,0,1}, font, 1.9)
		--TextDraw.DrawTextCentered("Zeskanuj swój Kod Kreskowy", SCREEN_WIDTH/2, currentY, {0.27,0.84,0.43,1}, font, 2.3)
	elseif state == "SSF" then
        -- Draw barcode screen elements with no fade effect
		love.graphics.setColor(1, 1, 1, 1)
		love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		love.graphics.setColor(0.27,0.84,0.43,1)
		love.graphics.print("->", font, -barY + 30, (promka_sel2 * 26 ) + 10, 0, 3)
		TextDraw.DrawTextCentered("Festiwal Kuponów!", SCREEN_WIDTH/2, -barY + 45, {0.27,0.84,0.43, 1}, font, 3.2)
		TextDraw.DrawTextCentered(responded[1].content.name, SCREEN_WIDTH/2 + barY - 20, 60, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered(responded[2].content.name, SCREEN_WIDTH/2 + barY - 20, 85, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered(responded[3].content.name, SCREEN_WIDTH/2 + barY - 20, 110, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered(responded[4].content.name, SCREEN_WIDTH/2 + barY - 20, 135, {0.27,0.84,0.43, 1}, font, 1.9)
	elseif state == "bierzlubnie" then
        -- Draw barcode screen elements with no fade effect
		love.graphics.setColor(1, 1, 1, 1)
		love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		love.graphics.setColor(0.27,0.84,0.43,1)
		TextDraw.DrawTextCentered(responded[promka_sel2].content.name, SCREEN_WIDTH/2, 35, {0.27,0.84,0.43, 1}, font, 3.2)
		local opis = limitchar(responded[promka_sel2].content.description) .. "\n \n Cały Opis jest Dostępny w Aplikacji Żappka :)"
		love.graphics.printf(opis, 5, 50, 250, "center", 0, 1.55, 1.55)
		love.graphics.printf("Kup za " .. responded[promka_sel2].content.requireRedeemedPoints .. " Żappsów", 5, 175, 250, "center", 0, 1.55, 1.55)
		TextDraw.DrawTextCentered("Wciśnij Select by Aktywować Kupon", SCREEN_WIDTH/2, 205, {0.57,0.24,0.43, 1}, font, 2.2)
		love.graphics.setColor(1, 1, 1, 1)
	elseif state == "options" then
        -- Draw barcode screen elements with no fade effect
		love.graphics.setColor(1, 1, 1, 1)
		love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		love.graphics.setColor(0.27,0.84,0.43,1)
		love.graphics.print("->", font, 0, (option_sel * 26 ) + 10, 0, 3)
		TextDraw.DrawTextCentered("Opcje", SCREEN_WIDTH/2, 30, {0.27,0.84,0.43, 1}, font, 3.2)
		TextDraw.DrawTextCentered("Wczytywanie Obrazków: " .. optiontable[1], SCREEN_WIDTH/2, 60, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered("Alt. Kalibracja Kodów QR: " .. optiontable[2], SCREEN_WIDTH/2, 85, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered("Wymuś tryb Offline: " .. optiontable[3], SCREEN_WIDTH/2, 110, {0.27,0.84,0.43, 1}, font, 1.9)
    end
end
local function extract_p_tags(html)
    local paragraphs = {}
	if html:gmatch("<p>(.-)</p>") then
		for p in html:gmatch("<p>(.-)</p>") do
			table.insert(paragraphs, "<p>" .. p .. "</p>")
		end
	elseif html:gmatch("<p (.-)>(.-)</p>") then
		for p in html:gmatch("<p (.-)>(.-)</p>") do
			table.insert(paragraphs, "<p (.-)>" .. p .. "</p>")
		end
	end
    return table.concat(paragraphs, "\n")
end
function limitchar(str)
	if str:len() > 167 then
		return string.sub(str, 1, 167) .. "..."
	else
		return str
	end
end
function parsedesc(text_to_parse)
	local str = text_to_parse
	
	local output = str:gsub("<b>", ""):gsub("</b>", ""):gsub("<br/>", ""):gsub("<noscript>", ""):gsub("</noscript>", ""):gsub("<img.-/>", ""):gsub("<p>", ""):gsub("</p>", " "):gsub("<", ""):gsub("strong", ""):gsub("a href=[^>]+", ""):gsub("&amp;", ""):gsub("%(", ""):gsub("%)", ""):gsub("&nbsp;", " "):gsub("/%a", ""):gsub("%a/", ""):gsub("/", ""):gsub(">", "")
	
	return output
end
function draw_bottom_screen()
    SCREEN_WIDTH = 400
    SCREEN_HEIGHT = 240
    love.graphics.setColor(1, 1, 1, 1)
	if state ~= "bierzlubnie" then
		love.graphics.draw(bottomdark, 0, 0)
	else
		love.graphics.setColor(1, 1, 1, 1)
		love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		if optiontable[1] == "true" then
			love.graphics.draw(kuponimage, 10, -20, 0, 0.3, 0.3)
		end
    end
    love.graphics.setColor(0, 0, 0, 1)
	if state == "main_strona" or state == "barcode" then
		love.graphics.print("A - Zobacz swój kod", font, 20, 10, 0, 1.2)
		love.graphics.print("Y - Wygeneruj Ponownie Kod", font, 20, 25, 0, 1.2)
		love.graphics.print("X - Zmień swoją nazwe", font, 20, 40, 0, 1.2)
		love.graphics.print("Select - Opcje", font, 20, 55, 0, 1.2)
		love.graphics.setColor(1, 1, 1, 1)
	elseif state == "promki_sel" or state == "promki" then
		love.graphics.print("A - Obczaj Promocje", font, 20, 10, 0, 1.2)
		love.graphics.print("DPad Góra - w góre lol", font, 20, 25, 0, 1.2)
		love.graphics.print("DPad Dół - w dół lol", font, 20, 40, 0, 1.2)
		love.graphics.setColor(1, 1, 1, 1)
	end
end

function love.gamepadpressed(joystick, button)
	if state == "main_strona" or state == "options" then
		if button == "select" then
			if state == "options" then
				sfx2:play()
				state = "main_strona"
				isFading = false
			elseif state == "main_strona" then
				sfx:play()
				state = "options"
			end
		end
	end
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
		elseif state == "promki_sel" then
			image = false
			updatepromki(promki_table[promka_sel])
		elseif state == "promki" or state == "SSF" then
			if optiontable[1] == "true" then
				if love._potion_version == nil then
					png_acja()
				else
					t3x_acja()
				end
			end
			state = "bierzlubnie"
		elseif state == "options" then
			if optiontable[option_sel] == "true" then
				optiontable[option_sel] = "false"
			elseif optiontable[option_sel] == "false" then
				optiontable[option_sel] = "true"
			end
        end
    end
    if button == "start" then
        love.event.quit()
    end
	if state == "barcode" then
		if button == "y" then
			calculatetotp()
		end
	end
	if button == "x" then
        changename()
    end
	if optiontable[3] == "false" then
		if button == "rightshoulder" then
			if state == "promki_sel" or state == "promki" or state == "bierzlubnie" then
				sfx2:play()
				state = "main_strona"
				--isFading = false
			elseif state == "main_strona" then
				sfx:play()
				--updatepromki()
				state = "promki_sel" 
				--isScrolling = true
				--isFading = true
				isScrolling = true
				isFading = true
				elapsedTime = 0  -- Reset elapsed time for scrolling animation
				elapsedTimeFade = 0
			end
		end
		if button == "leftshoulder" then
			if state == "SSF" or state == "promki" or state == "bierzlubnie" then
				sfx2:play()
				state = "main_strona"
				--isFading = false
			elseif state == "main_strona" then
				sfx:play()
				updatessf()
				limit = #responded
				state = "SSF"
				--isScrolling = true
				--isFading = true
				isScrolling = true
				isFading = true
				elapsedTime = 0  -- Reset elapsed time for scrolling animation
				elapsedTimeFade = 0
			end
		end
	end
	if state == "options" then
		if button == "dpdown" then
			if option_sel < 3 then
				option_sel = option_sel + 1
			end
		end
		if button == "dpup" then
			if option_sel > 1 then
				option_sel = option_sel - 1
			end
		end
	end
	if state == "promki_sel" then
		if button == "dpdown" then
			if promka_sel < 5 then
				promka_sel = promka_sel + 1
			end
		end
		if button == "dpup" then
			if promka_sel > 1 then
				promka_sel = promka_sel - 1
			end
		end
	end
	if state == "promki" or state == "SSF" then
		if button == "dpdown" then
			if promka_sel2 < limit then
				promka_sel2 = promka_sel2 + 1
			end
		end
		if button == "dpup" then
			if promka_sel2 > 1 then
				promka_sel2 = promka_sel2 - 1
			end
		end
	end
	if state == "bierzlubnie" then
		if button == "back" then
			if niedlapsakurwa == false then
				local uuid = responded[promka_sel2].content.uuid
				dawajmito(uuid, false)
			else
				local uuid = responded[promka_sel2].content.uuid
				dawajmito(uuid, true)
			end
		end
	end
end
function dawajmito(uuid_value, spowrotem)
	jsonread = false
	if spowrotem == false then
		local data = json.encode({key = "uuid", uuid_value = value})
		refresh_data("https://zabka-snrs.zabka.pl/v4/promotions/promotion/batch-activate", data, {["api-version"] = "4.4", ["application-id"] = "%C5%BCappka", ["user-agent"] = "Synerise Android SDK 5.9.0 pl.zabka.apb2c", ["accept"] = "application/json", ["mobile-info"] = "horizon;28;AW700000000;9;CTR-001;nintendo;5.9.0", ["content-type"] = "application/json; charset=UTF-8", ["authorization"] = authtoken}, "GET")
	else
        local data = json.encode({key = "uuid", uuid_value = value})
		refresh_data("https://zabka-snrs.zabka.pl/v4/promotions/promotion/batch-deactivate", data, {["api-version"] = "4.4", ["application-id"] = "%C5%BCappka", ["user-agent"] = "Synerise Android SDK 5.9.0 pl.zabka.apb2c", ["accept"] = "application/json", ["mobile-info"] = "horizon;28;AW700000000;9;CTR-001;nintendo;5.9.0", ["content-type"] = "application/json; charset=UTF-8", ["authorization"] = authtoken}, "GET")	
	end
end
function alt_kalibracja()
	local lastserverczas = love.filesystem.read("LastCzasInternet.txt")
	local lastlocalczas = love.filesystem.read("LastCzasIntranet.txt")
	local currentlocalczas = os.time()
	local dawajczas = lastserverczas + (currentlocalczas - lastlocalczas)
	return dawajczas
end
function updatetime_withserver()
	local data = ""
	refresh_data("https://zabka-snrs.zabka.pl/v4/server/time", data, {["api-version"] = "4.4", ["application-id"] = "%C5%BCappka", ["user-agent"] = "Synerise Android SDK 5.9.0 pl.zabka.apb2c", ["accept"] = "application/json", ["mobile-info"] = "horizon;28;AW700000000;9;CTR-001;nintendo;5.9.0", ["content-type"] = "application/json; charset=UTF-8", ["authorization"] = authtoken}, "GET")
	local dawajczas = isoToUnix(responded.serverTime) 
	love.filesystem.write("LastCzasInternet.txt", dawajczas)
	love.filesystem.write("LastCzasIntranet.txt", os.time())
	return dawajczas
end
function t3x_acja()
	local data = json.encode({url = responded[promka_sel2].content.images[1].url})
	image = true
	refresh_data("https://api.szprink.xyz/t3x/convert", data, {["api-version"] = "4.4", ["application-id"] = "%C5%BCappka", ["user-agent"] = "Synerise Android SDK 5.9.0 pl.zabka.apb2c", ["accept"] = "application/json", ["mobile-info"] = "horizon;28;AW700000000;9;CTR-001;nintendo;5.9.0", ["content-type"] = "application/json"}, "POST")
	kuponimageData = love.filesystem.write("temp.t3x", imagebody)
	kuponimage = love.graphics.newImage("temp.t3x")
end
function png_acja()
	local data = ""
	image = true
	refresh_data(responded[promka_sel2].content.images[1].url, data, {}, "GET")
	kuponimageData = love.filesystem.write("temp.png", imagebody)
	kuponimage = love.graphics.newImage("temp.png")
end
function updatezappsy()
	local data = ""
	refresh_data("https://zabka-snrs.zabka.pl/schema-service/v2/documents/points/generate", data, {["api-version"] = "4.4", ["application-id"] = "%C5%BCappka", ["user-agent"] = "Synerise Android SDK 5.9.0 pl.zabka.apb2c", ["accept"] = "application/json", ["mobile-info"] = "horizon;28;AW700000000;9;CTR-001;nintendo;5.9.0", ["content-type"] = "application/json; charset=UTF-8", ["authorization"] = authtoken}, "GET")
end
function updatessf()
	local data = ""
	refresh_data("https://zabka-snrs.zabka.pl/schema-service/proxy/promotions?page=1&limit=20&tagNames=SSF_kupony&sort=priority%2Cdesc", data, {["api-version"] = "4.4", ["authorization"] = "Bearer " .. authtoken, ["content-type"] = "application/json", ["accept"] = "application/json", ["user-agent"] = "okhttp/4.12.0"}, "GET")
end
function updatepromki(endlol)
	local data = ""
	state = "promki"
	print(endlol)
	refresh_data("https://zabka-snrs.zabka.pl/schema-service/proxy/promotions?page=1&limit=20&type=CUSTOM&status=ASSIGNED%2CACTIVE&tagNames=" .. endlol .. "&sort=status%2Casc&sort=priority%2Cdesc", data, {["api-version"] = "4.4", ["authorization"] = "Bearer " .. authtoken, ["content-type"] = "application/json", ["accept"] = "application/json", ["user-agent"] = "okhttp/4.12.0"}, "GET")
	limit = #responded
end
function tel_login()
	if love._potion_version == nil then
		if gui_design_mode == false then
			handle_authflow()
			numertel = "numer_tel"
			sendvercode(numertel)
			test()
		end
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
		loadingtext = "Logowanie..."
		state = "loading"
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
	if gui_design_mode == false then
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
		refresh_data("https://zabka-snrs.zabka.pl/sauth/v3/auth/login/client/conditional", data, {["api-version"] = "4.4", ["application-id"] = "%C5%BCappka", ["user-agent"] = "Synerise Android SDK 5.9.0 pl.zabka.apb2c", ["accept"] = "application/json", ["mobile-info"] = "horizon;28;AW700000000;9;CTR-001;nintendo;5.9.0", ["content-type"] = "application/json; charset=UTF-8"}, "POST")
		loadingtext = "Logowanie 65%..."
		authtoken = responded.token
		local data = ""
		refresh_data("https://qr-bff.spapp.zabka.pl/qr-code/secret", data, {["authorization"] = "Bearer " .. tokentemp, ["content-type"] = "application/json", ["accept"] = "application/json", ["app"] = "zappka-mobile", ["user-agent"] = "okhttp/4.12.0", ["content-length"] = "0"}, "GET")
		id = responded.userId
		love.filesystem.write("secret.hex.txt", responded.secrets.loyal)
		love.filesystem.write("id.txt", id)
		love.filesystem.write("token.txt", authtoken)
		updatezappsy()
		zappsy = responded.content.points
		calculatetotp()
	end
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
    -- Update the timer
    timer = timer - dt
	
    love.graphics.origin()  
end