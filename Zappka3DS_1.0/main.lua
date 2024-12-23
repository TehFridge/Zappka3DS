require("lib.text-draw")

json = require("lib.json")
local qrcode = require('lib.loveqrcode')
local https = require("https")
local uuidgen = require("lib.uuid")
local ltn12 = require("ltn12")
local bit = require("bit")    
local sha1 = require("sha1")
local struct = require("lib.struct")
local reference = 0
local scrolltimerX = -100
local option_sel = 1
local promka_sel = 1
local qrcal = "true"
local wavelogo = "true"
local imageload = "true"
local promka_sel2 = 1
local SCREEN_WIDTH = 400 
local SCREEN_HEIGHT = 240
local lookup = {19.194377939831, 19.451234900763, 19.659473904451, 19.818574461834, 19.928138904377, 19.987893378032, 19.997688527736, 19.957499870716, 19.867427857684, 19.727697621764, 19.538658415776, 19.300782739285, 19.014665157599, 18.681020815666, 18.300683650581, 17.874604307181, 17.403847761927, 16.889590661017, 16.333118379383, 15.735821807925, 15.099193877004, 14.424825824899, 13.714403220536, 12.969701750444, 12.192582780467, 11.384988703313, 10.548938083588, 9.6865206124343, 8.7998918843924, 7.8912680095345, 6.9629200743458, 6.0171684651899, 5.0563770685517, 4.0829473625525, 3.0993124145047, 2.1079307995114, 1.1112804553084, 0.1118524887109, -0.88785505085677, -1.8853434151833, -2.8781194028397, -3.8637015908846, -4.8396265371292, -5.8034549374598, -6.7527777228276, -7.685222080667, -8.598457385691, -9.4902010252409, -10.358224104629, -11.200357018214, -12.014494872287, -12.798602746207, -13.550720778645, -14.268969066215, -14.951552362255, -15.596764564008, -16.202992976988, -16.768722345882, -17.292538641893, -17.773132597079, -18.209302976841, -18.599959582382, -18.944125975639, -19.240941919867, -19.489665529785, -19.689675125899, -19.840470788383, -19.941675606614, -19.993036621252, -19.994425456509, -19.945838641021, -19.847397616521, -19.699348434306, -19.502061140227, -19.256028849774, -18.961866515543, -18.620309390173, -18.232211188596, -17.7985419542, -17.32038563422, -16.798937370437, -16.235500511946, -15.631483357459, -14.988395635299, -14.307844729855, -13.591531663969, -12.841246847255, -12.058865601015, -11.246343470909, -10.405711339107, -9.5390703481479, -8.64858664917, -7.736485987666, -6.8050481402769, -5.8566012165376, -4.8935158398159, -3.9181992219885, -2.9330891466648, -1.9406478759978, -0.94335599631063, 0.1118524887109, 1.1112804553084, 2.1079307995114, 3.0993124145047, 4.0829473625525, 5.0563770685517, 6.0171684651899, 6.9629200743458, 7.8912680095345, 8.7998918843924, 9.6865206124343, 10.548938083588, 11.384988703313, 12.192582780467, 12.969701750444, 13.714403220536, 14.424825824899, 15.099193877004, 15.735821807925, 16.333118379383, 16.889590661017, 17.403847761927, 17.874604307181, 18.300683650581, 18.681020815666, 19.014665157599}
local image = false
local timer = 30
local exists = "dunno"
local intranet = "false"
local state = ""
local elapsedTime = 0
local scrollDuration = 0.5  
local startY = 400          
local endY = 210            
local startFade = 1          
local endFade = 0.5            
local startbarY = 300          
local endbarY = 15            
local isScrolling = false
local isFading = false
local currentY = startY
local banner = love.graphics.newImage("assets/banner.png")
local qrwatermark = love.graphics.newImage("assets/zappka_qr_logo.png")
local bottomdark = love.graphics.newImage("assets/logo.png")
local failed = love.graphics.newImage("assets/failed.png")
local success = love.graphics.newImage("assets/success.png")
local elapsedTimeFade = 0
local fadeDuration = 0.2
local stateChanged = false
local music = love.audio.newSource("bgm/bgm.ogg", "stream")
local sfx = love.audio.newSource("bgm/sfx.ogg", "static")
local sfx2 = love.audio.newSource("bgm/sfx2.ogg", "static")
local jsonread = true
local numSegments = 126 
local offsets = {}      
local offsetChannel = love.thread.getChannel("offsetChannel")
local buttons = {}
local sin = math.sin
local pi = math.pi
name = "blank"
codeforinput = "blank"
opcjeexist = love.filesystem.exists("opcje.lua")
loggedin = love.filesystem.exists("secret.hex.txt")
existsname = love.filesystem.exists("imie.txt")
love.graphics.setDefaultFilter("nearest")
gui_design_mode = false

if love._potion_version == nil then
	font = love.graphics.newFont("bold.ttf", 12, "normal", 4) 
	local nest = require("nest").init({ console = "3ds", scale = 1 })
	love._nest = true
    love._console_name = "3DS"
else
	font = love.graphics.newFont("bold.ttf", 13, "normal", 3.5) 
end
local timerIncrement = 1
local showredeemedtime = 11
local pagegap = 0
local imgWidth = bottomdark:getWidth()
local imgHeight = bottomdark:getHeight()
local sliceWidth = imgWidth / numSegments
local yPos = love.graphics.getHeight() / 2 - imgHeight / 2
local yOffsetIndex = 0

local quads = {}
local logoSpriteBatch = love.graphics.newSpriteBatch(bottomdark, numSegments)
for i = 0, numSegments - 1 do
    local x = i * sliceWidth
    quads[i + 1] = love.graphics.newQuad(x, 0, sliceWidth, imgHeight, imgWidth, imgHeight)
end
function loadTableFromFile(filename)
    local chunk = love.filesystem.load(filename)
    if chunk then
        return chunk() 
    else
        return nil
    end
end
local function getYOffsetIndex(i)
	return (yOffsetIndex + i) % 126 + 1 
end
local function updateYOffsetIndex() yOffsetIndex = (yOffsetIndex + 1) % 126 end
function love.load()
	authtoken = love.filesystem.read("token.txt")
    selectioncode = 1
	redeemedstatus = "default"
	APP_VER = "v1.2.2_apifix"
	if opcjeexist then
		local loadedTable = loadTableFromFile("opcje.lua")
		if loadedTable then
			optiontable = loadedTable
			print("Table loaded successfully!")
		else
			print("Failed to load the table.")
		end
	else 
		optiontable = {imageload, qrcal, wavelogo}
		saveTableToFile(optiontable, "opcje.lua")
	end
	generated_once = false
	if existsname == false then 
        name = "3DS"
	else 
	    name =  love.filesystem.read("imie.txt")
	end
	
    if loggedin == false then 
        codeforinput = "101010101010"
		jsonread = false
		authtoken = "kurwa"
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
		jsonread = false
		updatezappsy() 
		jsonread = true
		if code == 401 then
			refreshtokenplz()
			updatezappsy()
		end
		if not string.find(body, "points") then
			intranet = "true"
		else
			intranet = "false"
			jsonread = true
			checksoftban()
			update_ployzones()
			updatewallet()
			updatezappsy()
			zappsy = responded.data.loyaltyProgram.points
			-- if updatetime_withserver() < 1727128800 then
				-- promki_table = {"Prosto_z_Pieca", "kat_lody", "kat_napoje", "kat_wiekszyglod", "Na_kanapke", "kat_piwo"}
				-- promki_nametable = {"Streetfood", "Lody", "Napoje", "Głodny?", "Wszystko na Kanapkę.", "Może Piwko?"}
				-- SSF = true
				-- print("SSF true")
			-- else
				-- promki_table = {"Prosto_z_Pieca", "kat_lody", "kat_napoje", "kat_wiekszyglod", "Na_kanapke", "kat_piwo"}
				-- promki_nametable = {"Streetfood", "Lody", "Napoje", "Głodny?", "Wszystko na Kanapkę.", "Może Piwko?"}
				-- SSF = false
				-- print("SSF false")
			-- end
			-- refresh_data("https://zabka-snrs.zabka.pl/schema-service/v2/documents/specialoffersettings/generate", data, {["api-version"] = "4.4", ["application-id"] = "%C5%BCappka", ["user-agent"] = "Synerise Android SDK 5.9.0 pl.zabka.apb2c", ["accept"] = "application/json", ["mobile-info"] = "horizon;28;AW700000000;9;CTR-001;nintendo;5.9.0", ["content-type"] = "application/json; charset=UTF-8", ["authorization"] = authtoken}, "GET")
			-- if responded.content.alcohol_eligibility == "1" then
				piweczko = true
			-- end
			-- refresh_data("https://zabka-snrs.zabka.pl/schema-service/proxy/promotions?page=1&limit=20&type=CUSTOM&status=ASSIGNED%2CACTIVE&tagNames=kat_top&sort=priority%2Cdesc",  data, {["api-version"] = "4.4", ["authorization"] = "Bearer " .. authtoken, ["content-type"] = "application/json", ["accept"] = "application/json", ["user-agent"] = "okhttp/4.12.0"}, "GET")
			-- topki = responded
	    end
		state = "main_strona"
    end
    
	
	    
    table.insert(buttons, createButton(195, 195, "assets/qrbutton.png", barcodenmachen, "main_strona", "barcode"))
	--table.insert(buttons, createButton(190, 155, "assets/przelejkurwa.png", przelejen, "main_strona", "dupa"))
	if intranet == "false" then
		table.insert(buttons, createButton(5, 195, "assets/kuponybutton.png", kuponmachen, "main_strona", "promki_sel"))
		--table.insert(buttons, createButton(5, 195, "assets/kuponybuttondis.png", nimafornow, "main_strona", "nima"))
	end
	table.insert(buttons, createButton(275, 5, "assets/exit.png", exitenmachen, "promki", "bierzlubnie"))
	table.insert(buttons, createButton(5, 10, "assets/settings.png", optenmachen, "main_strona", "options"))
	table.insert(buttons, createButton(100, 100, "assets/wyloguj.png", logout, "dupa", "options"))
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

    
    local days = 0
    for y = 1970, year - 1 do
        days = days + (isLeapYear(y) and 366 or 365)
    end

    for m = 1, month - 1 do
        days = days + daysInMonth(m, year)
    end

    days = days + (day - 1)

    
    local totalSeconds = days * 86400 + hour * 3600 + min * 60 + sec

    return totalSeconds
end
function createButton(x, y, imagePath, callback, statename, secstatename, thrdstate)
    local image = love.graphics.newImage(imagePath) 
    return {
        x = x,
        y = y,
        width = image:getWidth(),
        height = image:getHeight(),
        image = image,
        callback = callback,
        draw = function(self)
            
			if state == statename or state == secstatename then
				love.graphics.draw(self.image, self.x, self.y)
			end
        end,
        isTouched = function(self, touchX, touchY)
            
            return touchX > self.x and touchX < self.x + self.width and
                   touchY > self.y and touchY < self.y + self.height
        end
    }
end
function serializeTable(tbl, depth)
    depth = depth or 0
    local output = "{\n"
    
    for key, value in pairs(tbl) do
        local formattedKey = type(key) == "string" and string.format("[%q]", key) or "[" .. key .. "]"
        
        if type(value) == "table" then
            output = output .. string.rep(" ", depth + 4) .. formattedKey .. " = " .. serializeTable(value, depth + 4) .. ",\n"
        else
            local formattedValue = type(value) == "string" and string.format("%q", value) or tostring(value)
            output = output .. string.rep(" ", depth + 4) .. formattedKey .. " = " .. formattedValue .. ",\n"
        end
    end
    
    output = output .. string.rep(" ", depth) .. "}"
    return output
end

function saveTableToFile(tbl, filename)
    local serializedData = "return " .. serializeTable(tbl)
    love.filesystem.write(filename, serializedData)
end
function calculatetotp()
	local javaIntMax = 2147483647

	local function c(arr, index)
		local result = 0
		for i = index, index + 3 do  
			result = bit.bor(bit.lshift(result, 8), bit.band(arr:byte(i), 0xFF))
		end
		return result
	end

	local secretHex = codeforinput
	local secret = (secretHex:gsub('..', function(hex)
		return string.char(tonumber(hex, 16))
	end))

	if intranet == "true" then
		czas = alt_kalibracja()
	else
		czas = updatetime_withserver()
		print("internet: " .. czas)
	end

	ts = math.floor(czas / 30)
	print(ts)

	local msg = struct.pack(">L8", ts)
	local outputBytes = sha1.hmac_binary(secret, msg)

	if outputBytes ~= nil then
		
		if #outputBytes >= 4 then
			
			local byteIndex = outputBytes:byte(#outputBytes)
			local offset = bit.band(byteIndex, 15)
			print("byteIndex: " .. byteIndex)
			print("offset: " .. offset)

			
			local magicNumber = bit.band(c(outputBytes, offset + 1), javaIntMax) % 1000000  
			totp = string.format("%06d", magicNumber)
			print(totp)
			print("https://srln.pl/view/dashboard?ploy=" .. id .. "&loyal=" .. totp)
			qr1 = qrcode("https://srln.pl/view/dashboard?ploy=" .. id .. "&loyal=" .. totp)
			generated_once = true
		else
			print("outputBytes too short: " .. #outputBytes)
			generated_once = false
		end
	else
		print("Failed to generate HMAC")
		generated_once = false
		qr1 = nil
	end
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
    print(url)
	print(request)
	local request_body = request 
    response_body = {}
    
	if image == false then
		code, body, headers = https.request(url, {data = request_body, method = metoda, headers = inheaders})
	else
		code, imagebody, headers = https.request(url, {data = request_body, method = metoda, headers = inheaders})
	end
	
	print(code)
	print(body)
	if jsonread == true then
		responded = json.decode(body)
	end
end

function refreshtokenplz()
	jsonread = true
	mentos = love.filesystem.read("refresh.txt")
	local data = json.encode({grantType = "refresh_token", refreshToken = mentos})
	refresh_data("https://securetoken.googleapis.com/v1/token?key=AIzaSyDe2Fgxn_8HJ6NrtJtp69YqXwocutAoa9Q", data, {["content-type"] = "application/json"}, "POST")
	mentos = responded.refresh_token
	authtoken = responded.access_token
	love.filesystem.write("refresh.txt", mentos)
	love.filesystem.write("token.txt", authtoken)
	print("Token Refreshed!")
end

function nimafornow()
	if state == "nima" then
		sfx2:play()
		state = "main_strona"
	elseif state == "main_strona" then
		sfx:play()
		state = "nima"
	end
end

function draw_top_screen(dt)
    if isScrolling == true then
        
        progress = math.min(elapsedTime / scrollDuration, 1)
		fadedprogress = math.min(elapsedTimeFade / fadeDuration, 1)
		fadedProgres = 1 - (1 - fadedprogress) ^ 2  
		currentFade = startFade + (endFade - startFade) * fadedProgres

        
        easedProgress = 1 - (1 - progress) ^ 2  

        
        currentY = startY + (endY - startY) * easedProgress
    	barY = startbarY + (endbarY - startbarY) * easedProgress
    end

    
    if state == "main_strona" then
        
		love.graphics.setColor(1, 1, 1, 1)
        love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		TextDraw.DrawTextCentered('Cześć, ' .. name, SCREEN_WIDTH/2, 45, {0.27,0.84,0.43, 1}, font, 4.5)
		love.graphics.print(APP_VER, font, 5, 205, 0, 2, 2)
		love.graphics.setColor(1,1,1,1)
		if optiontable[3] == "true" then
			logoSpriteBatch:clear()
			for i = 0, numSegments - 1 do
				local yOffset = (lookup[i + 1] or 0) / 2  
			
				logoSpriteBatch:add(quads[i + 1], i*sliceWidth, yPos + yOffset)
			end    
				
			love.graphics.draw(logoSpriteBatch)
		else
			love.graphics.draw(bottomdark, 0,0)
		end
		if showredeemedtime < 10 then
			if redeemedstatus == "success" then
				love.graphics.draw(success, 0, 0)
			else
				love.graphics.draw(failed, 0, 0)
			end
		end
    elseif state == "barcode" then
        
		love.graphics.setColor(1, 1, 1, currentFade)
		love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		love.graphics.setColor(1, 1, 1, 1)
		love.graphics.rectangle("fill", 5, barY - 10, SCREEN_WIDTH - 10, SCREEN_HEIGHT, 6, 6, 4)
		love.graphics.setColor(0, 0, 0, 1)
		love.graphics.rectangle("line", 5, barY - 10, SCREEN_WIDTH - 10, SCREEN_HEIGHT, 6, 6, 4)
		love.graphics.setColor(1, 1, 1, 1)
		love.graphics.setColor(0, 0, 0, 1)
		if generated_once == true then
			if qr1 then
				qr1:draw(90,barY - 3,0,5)
				love.graphics.setColor(1, 1, 1, 1)
				love.graphics.draw(qrwatermark, 0,barY - 10)
			else
				love.graphics.printf("Coś poszło nie tak!", font, 5, barY + 90, 250, "center", 0, 1.55, 1.55)
				love.graphics.printf("Spróbuj ponownie póżniej.", font, 5, barY + 110, 250, "center", 0, 1.55, 1.55)
			end
		else
			love.graphics.printf("Wygeneruj Kod QR", font, 5, barY + 90, 250, "center", 0, 1.55, 1.55)
			love.graphics.printf("Wciśnij Y", font, 5, barY + 110, 250, "center", 0, 1.55, 1.55)
		end
		love.graphics.setColor(0.27,0.84,0.43,currentFade)
	elseif state == "login" then
        
		love.graphics.setColor(1, 1, 1, 1)
        love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		TextDraw.DrawTextCentered('Zaloguj się Numerem Telefonu', 200, 80, {0.27,0.84,0.43,1}, font, 2.7)
		TextDraw.DrawTextCentered('Wciśnij A', 200, 140, {0.27,0.84,0.43,1}, font, 3)
	elseif state == "smscode" then
		love.graphics.setColor(1, 1, 1, 1)
        love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		TextDraw.DrawTextCentered('By wprowadzić kod SMS', 200, 80, {0.27,0.84,0.43,1}, font, 2.7)
		TextDraw.DrawTextCentered('Wciśnij A', 200, 140, {0.27,0.84,0.43,1}, font, 3)
	elseif state == "restartplz" then
		love.graphics.setColor(1, 1, 1, 1)
        love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		TextDraw.DrawTextCentered('Zrestartuj Aplikacje', 200, 80, {0.27,0.84,0.43,1}, font, 2.7)
		TextDraw.DrawTextCentered('(nic się nie zjebało) Wciśnij Start', 200, 140, {0.27,0.84,0.43,1}, font, 2)
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
        
		love.graphics.setColor(1, 1, 1, 1)
		love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		love.graphics.setColor(0.27,0.84,0.43,1)
		TextDraw.DrawTextCentered(currentpagename, SCREEN_WIDTH/2, -barY + 45, {0.27,0.84,0.43, 1}, font, 3.2)
		TextDraw.DrawText("->", 5, 50 + selectioncode * 20, {0.27,0.84,0.43, 1}, font, 1.9, true)
		if category ~= true then
			kurwalimit = #currentpromkitable.data.ployOfferListing.ployOffers
			if #currentpromkitable.data.ployOfferListing.ployOffers < 6 then
				for i = 1, #currentpromkitable.data.ployOfferListing.ployOffers do 
					TextDraw.DrawText(currentpromkitable.data.ployOfferListing.ployOffers[i + pagegap].coupon.targetPromotion.name, 27, 50 + 20 * i, {0.27,0.84,0.43, 1}, font, 1.9, true)
				end
			else 
				for i = 1, 6 do
					TextDraw.DrawText(currentpromkitable.data.ployOfferListing.ployOffers[i + pagegap].coupon.targetPromotion.name, 27, 50 + 20 * i, {0.27,0.84,0.43, 1}, font, 1.9, true)
				end
			end
		else
			kurwalimit = #currentpromkitable.data.offerCategoryFeed.offerCategory.levels
			if #currentpromkitable.data.offerCategoryFeed.offerCategory.levels < 6 then
				for i = 1, #currentpromkitable.data.offerCategoryFeed.offerCategory.levels do 
					TextDraw.DrawText(currentpromkitable.data.offerCategoryFeed.offerCategory.levels[i + pagegap].title, 27, 50 + 20 * i, {0.27,0.84,0.43, 1}, font, 1.9, true)
				end
			else 
				for i = 1, 6 do
					TextDraw.DrawText(currentpromkitable.data.offerCategoryFeed.offerCategory.levels[i + pagegap].title, 27, 50 + 20 * i, {0.27,0.84,0.43, 1}, font, 1.9, true)
				end
			end
		end
    elseif state == "nima" then
		love.graphics.setColor(1, 1, 1, 1)
		love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		love.graphics.setColor(0.27,0.84,0.43,1)
		TextDraw.DrawTextCentered("Kupony z nowym API", SCREEN_WIDTH/2, 60, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered("not implemented lol", SCREEN_WIDTH/2, 85, {0.27,0.84,0.43, 1}, font, 1.9)
	elseif state == "promki_sel" or state == "category_sel" then
        
		love.graphics.setColor(1, 1, 1, 1)
		love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		love.graphics.setColor(0.27,0.84,0.43,1)
		--love.graphics.print("->", font, -barY + 50, (promka_sel * 26 ) + 10, 0, 3)
		TextDraw.DrawTextCentered("Kupony:", SCREEN_WIDTH/2, -barY + 45, {0.27,0.84,0.43, 1}, font, 3.2)
		-- TextDraw.DrawTextCentered(promki_nametable[1], SCREEN_WIDTH/2 + barY - 20, 60, {0.27,0.84,0.43, 1}, font, 1.9)
		-- TextDraw.DrawTextCentered(promki_nametable[2], SCREEN_WIDTH/2 + barY - 20, 85, {0.27,0.84,0.43, 1}, font, 1.9)
		-- TextDraw.DrawTextCentered(promki_nametable[3], SCREEN_WIDTH/2 + barY - 20, 110, {0.27,0.84,0.43, 1}, font, 1.9)
		-- TextDraw.DrawTextCentered(promki_nametable[4], SCREEN_WIDTH/2 + barY - 20, 135, {0.27,0.84,0.43, 1}, font, 1.9)
		-- TextDraw.DrawTextCentered(promki_nametable[5], SCREEN_WIDTH/2 + barY - 20, 160, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawText("->", 5, 50 + selectioncode * 20, {0.27,0.84,0.43, 1}, font, 1.9, true)
		if category ~= true then
			kurwalimit = #zabkazonefeed.data.zoneFeed.sections[1].tiles
			if #zabkazonefeed.data.zoneFeed.sections[1].tiles < 6 then
				for i = 1, #zabkazonefeed.data.zoneFeed.sections[1].tiles do 
					if zabkazonefeed.data.zoneFeed.sections[1].tiles[i + pagegap].offerTreeLevel.title ~= nil then
						TextDraw.DrawText(zabkazonefeed.data.zoneFeed.sections[1].tiles[i + pagegap].offerTreeLevel.title, -barY + 50, 50 + 20 * i, {0.27,0.84,0.43, 1}, font, 1.9, true)
					else
						TextDraw.DrawText(zabkazonefeed.data.zoneFeed.sections[1].tiles[i + pagegap].offerTreeLevel.name, -barY + 50, 50 + 20 * i, {0.27,0.84,0.43, 1}, font, 1.9, true)
					end
				end
			else 
				for i = 1, 6 do
					if zabkazonefeed.data.zoneFeed.sections[1].tiles[i + pagegap].offerTreeLevel.title ~= nil then
						TextDraw.DrawText(zabkazonefeed.data.zoneFeed.sections[1].tiles[i + pagegap].offerTreeLevel.title, -barY + 50, 50 + 20 * i, {0.27,0.84,0.43, 1}, font, 1.9, true)
					else
						TextDraw.DrawText(zabkazonefeed.data.zoneFeed.sections[1].tiles[i + pagegap].offerTreeLevel.name, -barY + 50, 50 + 20 * i, {0.27,0.84,0.43, 1}, font, 1.9, true)
					end
				end
			end
		else
			kurwalimit = #currentpromkitable.data.offerCategoryFeed.offerCategory.levels
			if #currentpromkitable.data.offerCategoryFeed.offerCategory.levels < 6 then
				for i = 1, #currentpromkitable.data.offerCategoryFeed.offerCategory.levels do 
					TextDraw.DrawText(currentpromkitable.data.offerCategoryFeed.offerCategory.levels[i + pagegap].title, 27, 50 + 20 * i, {0.27,0.84,0.43, 1}, font, 1.9, true)
				end
			else 
				for i = 1, 6 do
					TextDraw.DrawText(currentpromkitable.data.offerCategoryFeed.offerCategory.levels[i + pagegap].title, 27, 50 + 20 * i, {0.27,0.84,0.43, 1}, font, 1.9, true)
				end
			end
		end
		-- if piweczko == true then
			-- TextDraw.DrawTextCentered(promki_nametable[6], SCREEN_WIDTH/2 + barY - 20, 185, {0.27,0.84,0.43, 1}, font, 1.9)
		-- end
        
		
		
	elseif state == "SSF" then
        
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
        
		love.graphics.setColor(1, 1, 1, 1)
		love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		if optiontable[1] == "true" then
			love.graphics.draw(kuponimage, 60, -20, 0, 0.75, 0.75)
		end
	elseif state == "options" then
        
		love.graphics.setColor(1, 1, 1, 1)
		love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
		love.graphics.setColor(0.27,0.84,0.43,1)
		love.graphics.print("->", font, 0, (option_sel * 26 ) + 10, 0, 3)
		TextDraw.DrawTextCentered("Opcje", SCREEN_WIDTH/2, 30, {0.27,0.84,0.43, 1}, font, 3.2)
		TextDraw.DrawTextCentered("Wczytywanie Obrazków: " .. optiontable[1], SCREEN_WIDTH/2, 60, {0.27,0.84,0.43, 1}, font, 1.9)
		TextDraw.DrawTextCentered("Alt. Kalibracja Kodów QR: " .. optiontable[2], SCREEN_WIDTH/2, 85, {0.27,0.84,0.43, 1}, font, 1.9)
		love.graphics.print('Credits:', font, 0, 165, 0, 1.2, 1.2)
		love.graphics.print('Żappka3DS Theme - Ludwik Franke', font, 0, 180, 0, 1.2, 1.2)
		love.graphics.print('(https://soundcloud.com/ludwikfranke_crazy)', font, 0, 195, 0, 1.2, 1.2)
		love.graphics.print('Oprawa graficzna - layt_ (https://github.com/Laytdesu)', font, 0, 210, 0, 1.2, 1.2)
		love.graphics.print('Ikonki - Font Awesome (https://fontawesome.com/)', font, 0, 225, 0, 1.2, 1.2)
		TextDraw.DrawTextCentered("Falujące logo: " .. optiontable[3], SCREEN_WIDTH/2, 110, {0.27,0.84,0.43, 1}, font, 1.9)
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
function checksoftban()
	local data = json.encode({operationName = "CheckSoftBan",query = "query CheckSoftBan { userProfileStatus { banStatus { isSoftBanned } } }",variables = {}})
	local data = data:gsub('"variables":%[%]', '"variables":{}')
	refresh_data("https://api.spapp.zabka.pl/", data, {["user-agent"] = "Zappka/40038 (Horizon; nintendo/ctr; 56c41945-ba88-4543-a525-4e8f7d4a5812) REL/28", ["accept"] = "application/json", ["content-type"] = "application/json", ["authorization"] = "Bearer " .. authtoken}, "POST")
	if responded.data.userProfileStatus.banStatus.isSoftBanned == true then
		print("banned kurwa")
	else
		print("not banned fuck yea")
	end
end
function draw_bottom_screen()
    SCREEN_WIDTH = 400
    SCREEN_HEIGHT = 240
    love.graphics.setColor(1, 1, 1, 1)
	love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
	if loggedin == true then
		if intranet == "false" then
			TextDraw.DrawTextCentered('Ilość Żappsów: ' .. zappsy, 320/2, 25, {0.27,0.84,0.43, 1}, font, 2.1)
		else
			TextDraw.DrawTextCentered('Chwilowy Brak Żappsów', 320/2, 25, {0.27,0.84,0.43, 1}, font, 2.1)
			love.graphics.print('! Brak Dostępu do Internetu !', font, 0, 195, 0, 1.2, 1.2)
			love.graphics.print('Kody QR mogą się źle generować', font, 0, 210, 0, 1.2, 1.2)
			love.graphics.print('(nwm czas w 3dsach jakiś zjebany jest)', font, 0, 225, 0, 1.2, 1.2)
		end
		if state == "main_strona" then
			TextDraw.DrawText("Moje Kupony:", 27, 40, {0.27,0.84,0.43, 1}, font, 1.9, true)	
			if kurwalimit ~= 0 then
				TextDraw.DrawText("->", 5, 50 + selectioncode * 20, {0.27,0.84,0.43, 1}, font, 1.9, true)
				kurwalimit = #mojekupony.data.couponWallet.coupons
				if kurwalimit < 6 then
					for i = 1, kurwalimit do 
						--print(mojekupony.data.couponWallet.coupons[i + pagegap].__typename)
						if mojekupony.data.couponWallet.coupons[i + pagegap].singleCoupon ~= nil then
							TextDraw.DrawText(mojekupony.data.couponWallet.coupons[i + pagegap].singleCoupon.targetPromotion.name, 27, 50 + 20 * i, {0.27,0.84,0.43, 1}, font, 1.9, true)
						elseif mojekupony.data.couponWallet.coupons[i + pagegap].ployCoupon ~= nil then
							TextDraw.DrawText(mojekupony.data.couponWallet.coupons[i + pagegap].ployCoupon.targetPromotion.name, 27, 50 + 20 * i, {0.27,0.84,0.43, 1}, font, 1.9, true)
						end
						--print("Done")
					end
				else 
					for i = 1, 6 do
						if mojekupony.data.couponWallet.coupons[i + pagegap].singleCoupon ~= nil then
							TextDraw.DrawText(mojekupony.data.couponWallet.coupons[i + pagegap].singleCoupon.targetPromotion.name, 27, 50 + 20 * i, {0.27,0.84,0.43, 1}, font, 1.9, true)
						elseif mojekupony.data.couponWallet.coupons[i + pagegap].ployCoupon ~= nil then
							TextDraw.DrawText(mojekupony.data.couponWallet.coupons[i + pagegap].ployCoupon.targetPromotion.name, 27, 50 + 20 * i, {0.27,0.84,0.43, 1}, font, 1.9, true)
						end
					end
				end
			else 
				TextDraw.DrawText("Brak", 27, 70, {0.27,0.84,0.43, 1}, font, 1.9, true)	
			end
		elseif state == "promki_sel" or state == "promki" or state == "options" then
			TextDraw.DrawText("Góra/Dół - Wybierz         A - Zatwierdź" , 20, 35, {0,0,0,1}, font, 1.5)
		elseif state == "barcode" then
			TextDraw.DrawText("Y - Odśwież/Wygeneruj kod QR ponownie" , 26, 35, {0,0,0,1}, font, 1.3)
		elseif state == "bierzlubnie" then
			
			love.graphics.setColor(1, 1, 1, 1)
			love.graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
			love.graphics.setColor(0.27,0.84,0.43,1)
			TextDraw.DrawTextCentered(textname, 325/2, 35, {0.27,0.84,0.43, 1}, font, 2.1)
			love.graphics.printf(opis, 10, 50, 250, "center", 0, 1.2, 1.2)
			if kuponsingular ~= true then
				TextDraw.DrawTextCentered("Kup za " .. punktykurwa .. " Żappsów", 320/2, 175, {0.27,0.84,0.43, 1}, font, 1.8)
			end
		    if zappsy >= punktykurwa then
				if niedlapsakurwa == false then
					TextDraw.DrawTextCentered("Wciśnij Select by Aktywować Kupon", 320/2, 205, {0.57,0.24,0.43, 1}, font, 1.8)
				else
					TextDraw.DrawTextCentered("Wciśnij Select by Zwrócić Kupon", 320/2, 205, {0.57,0.24,0.43, 1}, font, 1.8)
				end
			else 
				TextDraw.DrawTextCentered("Masz za mało żappsów :(", 320/2, 205, {0.57,0.24,0.43, 1}, font, 1.8)
			end
			if kuponsingular ~= true then
				TextDraw.DrawTextCentered("Liczba odebranych kuponów: " .. numredeem, 320/2, 225, {0.27,0.84,0.43, 1}, font, 1.8)
			end
			love.graphics.setColor(1, 1, 1, 1)
		end
		love.graphics.setColor(0.27,0.84,0.43,1)
		for _, button in ipairs(buttons) do
			love.graphics.setColor(1, 1, 1, 1)
			button:draw()
		end
    end
	local memoryUsage = collectgarbage("count")
    
end
function logout()
	love.filesystem.remove("secret.hex.txt")
	love.filesystem.remove("token.txt")
	love.filesystem.remove("id.txt")
	state = "restartplz"
end
function exitenmachen()
	if state == "promki" or state == "bierzlubnie" then
		sfx2:play()
		selectioncode = 1
		pagegap = 0
		jsonread = true
		image = false
		state = "main_strona"
		category = false
	end
end
function kuponmachen()
	if state == "promki_sel" then
		sfx2:play()
		selectioncode = 1
		pagegap = 0
		jsonread = true
		image = false
		category = false
		state = "main_strona"
		
	elseif state == "main_strona" then
		sfx:play()
		selectioncode = 1
		pagegap = 0
		state = "promki_sel" 
		category = false
		isScrolling = true
		isFading = true
		elapsedTime = 0  
		elapsedTimeFade = 0
	end
end
function optenmachen()
	if state == "main_strona" or state == "options" then
		if state == "options" then
			sfx2:play()
			saveTableToFile(optiontable, "opcje.lua")
			state = "main_strona"
			isFading = false
		elseif state == "main_strona" then
			sfx:play()
			state = "options"
		end
	end
end
function barcodenmachen()
	if state == "barcode" then
		sfx2:play()
		state = "main_strona"
		isFading = false
	elseif state == "main_strona" then
		sfx:play()
		state = "barcode"
		isScrolling = true
		isFading = true
		elapsedTime = 0  
		elapsedTimeFade = 0
	end
end
function shiftRight(t)
    local last = t[#t]
    for i = #t, 2, -1 do
        t[i] = t[i-1]
    end
    t[1] = last
end
function love.touchpressed(id, x, y, dx, dy, pressure)
    
    for _, button in ipairs(buttons) do
        if button:isTouched(x, y) then
            button.callback() 
        end
    end
end
function love.gamepadpressed(joystick, button)
    if button == "a" then
		if state == "login" then
			tel_login()
		elseif state == "smscode" then
			smskod()
		elseif state == "promki_sel" or state == "category_sel" then
			image = false
			updatepromki()
		elseif state == "promki" or state == "SSF" or state == "main_strona" then
			if intranet == "false" then
				if state == "main_strona" and #mojekupony.data.couponWallet.coupons ~= 0 then
					if mojekupony.data.couponWallet.coupons[selectioncode + pagegap].__typename == "SingleCoupon" then
						if mojekupony.data.couponWallet.coupons[selectioncode + pagegap].singleCoupon.state ~= "ACTIVATED" then
							niedlapsakurwa = false
							print("nie aktywowany")
							--print(topki[promka_sel2 + 2].content.name)
						else
							niedlapsakurwa = true
							print("aktywowany")
							--print(topki[promka_sel2 + 2].content.name)
						end
						if optiontable[1] == "true" then
							if love._potion_version == nil then
								png_acja()
							else
								t3x_acja()
							end
						end
						textname = mojekupony.data.couponWallet.coupons[selectioncode + pagegap].singleCoupon.targetPromotion.name
						opis = "Kupon"
						punktykurwa = 0
						numredeem = mojekupony.data.couponWallet.coupons[selectioncode + pagegap].singleCoupon.possibleRedeems
						uuid = mojekupony.data.couponWallet.coupons[selectioncode + pagegap].singleCoupon.id
						kuponsingular = true
					elseif mojekupony.data.couponWallet.coupons[selectioncode + pagegap].__typename == "StackPloyCoupon" then
						if mojekupony.data.couponWallet.coupons[selectioncode + pagegap].ployCoupon.state ~= "ACTIVATED" then
							niedlapsakurwa = false
							print("nie aktywowany")
							--print(topki[promka_sel2 + 2].content.name)
						else
							niedlapsakurwa = true
							print("aktywowany")
							--print(topki[promka_sel2 + 2].content.name)
						end
						if optiontable[1] == "true" then
							if love._potion_version == nil then
								png_acja()
							else
								t3x_acja()
							end
						end
						textname = mojekupony.data.couponWallet.coupons[selectioncode + pagegap].ployCoupon.targetPromotion.name
						opis = "Kupon"
						punktykurwa = 0
						numredeem = mojekupony.data.couponWallet.coupons[selectioncode + pagegap].ployCoupon.possibleRedeems
						uuid = mojekupony.data.couponWallet.coupons[selectioncode + pagegap].ployCoupon.id
						kuponsingular = true
					end
					state = "bierzlubnie"
				elseif state == "promki" then
					local data = json.encode({operationName = "PloyOfferDetails", query = "query PloyOfferDetails($ployOfferId: ID!) { ployOffer(ployOfferId: $ployOfferId) { ployOffer { __typename ...PloyOfferParts id } details { __typename ...RichTextWithHeaderParts } } }  fragment PriceParts on Price { amount currencyCode fractionDigits }  fragment ColorParts on ThemeColor { light dark }  fragment BadgeParts on PromotionBadge { key label textColor { __typename ...ColorParts } }  fragment AbsoluteDiscountValueParts on AbsoluteDiscountValue { absoluteValue: value { __typename ...PriceParts } absoluteDiscount: discount { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment RelativeDiscountValueParts on RelativeDiscountValue { percentage relativeValue: value { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment MultibuyDiscountValueParts on MultibuyDiscountValue { multibuyValue: value { __typename ...PriceParts } maxQuantity triggerQuantity multibuyDiscount: discount { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } } }  fragment DiscountParts on DiscountValue { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } ... on MultibuyDiscountValue { __typename ...MultibuyDiscountValueParts } }  fragment HappyHourParts on HappyHour { alert validFrom validUntil }  fragment PromotionHighlightParts on PromotionHighlight { layout keyVisualImage { url } }  fragment TagParts on PromotionTag { backgroundColor { __typename ...ColorParts } key label longLabel textColor { __typename ...ColorParts } }  fragment ProductPromotionParts on ProductPromotion { id name image { url } detailsImage { url } contents detailsContents exclusivity isPricePerUnit hasLegalDetails hidePromotionAlerts legalShortDetails legalLongDetails validFrom validUntil promotionDurationDetails alternativeBasePrice { __typename ...PriceParts } basePrice { __typename ...PriceParts } badges { __typename ...BadgeParts } discount { __typename ...DiscountParts } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } tags { __typename ...TagParts } __typename }  fragment DigitalProductPromotionParts on DigitalProductPromotion { id name code contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment PartnerProductPromotionParts on PartnerProductPromotion { id name contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment InAppProductPromotionParts on InAppProductPromotion { id name contents detailsContents happyHour { __typename ...HappyHourParts } productType image { url } validFrom validUntil }  fragment CouponParts on Coupon { id keyVisualImage { url } lastingAt validFrom validUntil showFullDate targetPromotion { __typename ... on ProductPromotion { __typename ...ProductPromotionParts id } ... on DigitalProductPromotion { __typename ...DigitalProductPromotionParts } ... on PartnerProductPromotion { __typename ...PartnerProductPromotionParts } ... on InAppProductPromotion { __typename ...InAppProductPromotionParts } } currentRedeemedQuantity possibleRedeems redeemLimitPerClient state couponSource __typename }  fragment PloyOfferParts on PloyOffer { id coupon { __typename ...CouponParts id } price { base discounted } visualDiscount __typename }  fragment RichTextWithHeaderParts on RichTextWithHeader { header richText }", variables = { ployOfferId = currentpromkitable.data.ployOfferListing.ployOffers[selectioncode + pagegap].id}})
					refresh_data("https://api.spapp.zabka.pl/", data, {["user-agent"] = "Zappka/40038 (Horizon; nintendo/ctr; 56c41945-ba88-4543-a525-4e8f7d4a5812) REL/28", ["accept"] = "application/json", ["content-type"] = "application/json", ["authorization"] = "Bearer " .. authtoken}, "POST")
					if responded.data.ployOffer.ployOffer.coupon.currentRedeemedQuantity <= 0 then
						niedlapsakurwa = false
						print("nie aktywowany")
						--print(responded[promka_sel2].content.name)
					else
						niedlapsakurwa = true
						print("aktywowany")
						--print(responded[promka_sel2].content.name)
					end
					if optiontable[1] == "true" then
						if love._potion_version == nil then
							png_acja()
						else
							t3x_acja()
						end
					end
					textname = currentpromkitable.data.ployOfferListing.ployOffers[selectioncode + pagegap].coupon.targetPromotion.name
					opis = limitchar(responded.data.ployOffer.details[1].richText) .. "\n \n Cały Opis jest Dostępny w Aplikacji Żappka :)"
					punktykurwa = responded.data.ployOffer.ployOffer.price.base
					numredeem = responded.data.ployOffer.ployOffer.coupon.currentRedeemedQuantity
					uuid = responded.data.ployOffer.ployOffer.coupon.id
					kuponsingular = false
					state = "bierzlubnie"
				end
			end
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
	if intranet == "false" then
		if button == "rightshoulder" then
			-- if state == "promki_sel" or state == "promki" or state == "bierzlubnie" then
				-- sfx2:play()
				-- jsonread = true
				-- image = false
				-- state = "main_strona"
				
			-- elseif state == "main_strona" then
				-- sfx:play()
				
				-- state = "promki_sel" 
				
				
				-- isScrolling = true
				-- isFading = true
				-- elapsedTime = 0  
				-- elapsedTimeFade = 0
			-- end
			nimafornow()
		end
		if button == "leftshoulder" then
			if state == "main_strona" then
				image = false
				jsonread = true
				updatezappsy()
				zappsy = responded.data.loyaltyProgram.points
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
	if state == "promki_sel" or state == "promki" or state == "category_sel" or state == "main_strona" then
		if button == "dpup" then
			if kurwalimit < 6 then
				if selectioncode ~= 1 then
					selectioncode = selectioncode - 1
				end
			else 	
				if selectioncode ~= 1 then
					selectioncode = selectioncode - 1
				elseif (selectioncode + pagegap) ~= 1 and selectioncode >= 1 then
					pagegap = pagegap - 1
				end
			end
		end
		if button == "dpdown" then
			if kurwalimit < 6 then
				if selectioncode ~= kurwalimit then
					selectioncode = selectioncode + 1
				end
			else 	
				if selectioncode ~= 6 then
					selectioncode = selectioncode + 1
				elseif (selectioncode + pagegap) ~= kurwalimit and selectioncode >= 6 then
					pagegap = pagegap + 1
				end
			end
		end
	end
	-- if state == "promki" or state == "SSF" or state == "main_strona" then
		-- if button == "dpdown" then
			-- if promka_sel2 < limit then
				-- promka_sel2 = promka_sel2 + 1
			-- end
		-- end
		-- if button == "dpup" then
			-- if promka_sel2 > 1 then
				-- promka_sel2 = promka_sel2 - 1
			-- end
		-- end
	-- end
	if state == "bierzlubnie" then
		if button == "back" then
			if zappsy >= punktykurwa then
				if niedlapsakurwa == false then
					dawajmito(uuid, false)
					state = "main_strona"
				else
					dawajmito(uuid, true)
					state = "main_strona"
				end
			end
		end
	end
end
function dawajmito(uuid_value, spowrotem)
	if spowrotem == false then
		jsonread = false
		local data = json.encode({operationName = "ActivateCoupon",query = "mutation ActivateCoupon($activateCouponInput: ActivateCouponInput!) { activateCoupon(activateCouponInput: $activateCouponInput) { __typename } }",variables = {activateCouponInput = {amount = 1,couponId = uuid}}})
		refresh_data("https://api.spapp.zabka.pl/", data, {["user-agent"] = "Zappka/40038 (Horizon; nintendo/ctr; 56c41945-ba88-4543-a525-4e8f7d4a5812) REL/28", ["accept"] = "application/json", ["content-type"] = "application/json", ["authorization"] = "Bearer " .. authtoken}, "POST")
		successcode = code
		if successcode == 200 then
			print("success act")
			redeemedstatus = "success"
		else
			print("failed act")
			redeemedstatus = "failed"
		end
		image = false
		jsonread = true
		--refresh_data("https://zabka-snrs.zabka.pl/v4/promotions/promotion/get-item-for-client/uuid/" .. uuid_value, "", {["api-version"] = "4.4", ["application-id"] = "%C5%BCappka", ["user-agent"] = "Synerise Android SDK 5.9.0 pl.zabka.apb2c", ["accept"] = "application/json", ["mobile-info"] = "horizon;28;AW700000000;9;CTR-001;nintendo;5.9.0", ["content-type"] = "application/json; charset=UTF-8", ["authorization"] = authtoken}, "GET")
		updatezappsy()
		zappsy = responded.data.loyaltyProgram.points
		timerIncrement = 1
		showredeemedtime = 0
		updatewallet()
	else
		jsonread = false
        local data = json.encode({operationName = "DeactivateCoupon",query = "mutation DeactivateCoupon($input: DeactivateCouponInput!) { deactivateCoupon(deactivateCouponInput: $input) { __typename } }",variables = {input = {couponId = uuid}}})
		refresh_data("https://api.spapp.zabka.pl/", data, {["user-agent"] = "Zappka/40038 (Horizon; nintendo/ctr; 56c41945-ba88-4543-a525-4e8f7d4a5812) REL/28", ["accept"] = "application/json", ["content-type"] = "application/json", ["authorization"] = "Bearer " .. authtoken}, "POST")
		successcode = code
		if successcode == 200 then
			print("success deact")
			redeemedstatus = "success"
		else
			print("failed deact")
			redeemedstatus = "failed"
		end
		image = false
		jsonread = true
		--refresh_data("https://zabka-snrs.zabka.pl/v4/promotions/promotion/get-item-for-client/uuid/" .. uuid_value, "", {["api-version"] = "4.4", ["application-id"] = "%C5%BCappka", ["user-agent"] = "Synerise Android SDK 5.9.0 pl.zabka.apb2c", ["accept"] = "application/json", ["mobile-info"] = "horizon;28;AW700000000;9;CTR-001;nintendo;5.9.0", ["content-type"] = "application/json; charset=UTF-8", ["authorization"] = authtoken}, "GET")
		updatezappsy()
		zappsy = responded.data.loyaltyProgram.points
		timerIncrement = 1
		showredeemedtime = 0
		updatewallet()
	end
end
function alt_kalibracja()
	local lastserverczas = love.filesystem.read("LastCzasInternet.txt")
	local lastlocalczas = love.filesystem.read("LastCzasIntranet.txt")
	local currentlocalczas = os.time()
	local dawajczas = lastserverczas + (currentlocalczas - lastlocalczas)
	love.filesystem.write("LastCzasInternet.txt", dawajczas)
	love.filesystem.write("LastCzasIntranet.txt", os.time())
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
	if state == "main_strona" then
		if mojekupony.data.couponWallet.coupons[selectioncode + pagegap].__typename == "SingleCoupon" then
			local urlt3x = mojekupony.data.couponWallet.coupons[selectioncode + pagegap].singleCoupon.targetPromotion.image.url
			local urlt3x = urlt3x:gsub('webp', 'png')
			local data = json.encode({url = urlt3x})
			image = true
			refresh_data("https://api.szprink.xyz/t3x/convert", data, {["api-version"] = "4.4", ["application-id"] = "%C5%BCappka", ["user-agent"] = "Synerise Android SDK 5.9.0 pl.zabka.apb2c", ["accept"] = "application/json", ["mobile-info"] = "horizon;28;AW700000000;9;CTR-001;nintendo;5.9.0", ["content-type"] = "application/json"}, "POST")
			local imageData = love.image.newImageData(love.filesystem.newFileData(imagebody, "image.t3x"))
			kuponimage = love.graphics.newImage(imageData)
		elseif mojekupony.data.couponWallet.coupons[selectioncode + pagegap].__typename == "StackPloyCoupon" then
			local urlt3x = mojekupony.data.couponWallet.coupons[selectioncode + pagegap].ployCoupon.targetPromotion.image.url
			local urlt3x = urlt3x:gsub('webp', 'png')
			local data = json.encode({url = urlt3x})
			image = true
			refresh_data("https://api.szprink.xyz/t3x/convert", data, {["api-version"] = "4.4", ["application-id"] = "%C5%BCappka", ["user-agent"] = "Synerise Android SDK 5.9.0 pl.zabka.apb2c", ["accept"] = "application/json", ["mobile-info"] = "horizon;28;AW700000000;9;CTR-001;nintendo;5.9.0", ["content-type"] = "application/json"}, "POST")
			local imageData = love.image.newImageData(love.filesystem.newFileData(imagebody, "image.t3x"))
			kuponimage = love.graphics.newImage(imageData)
		end
	else
		local urlt3x = responded.data.ployOffer.ployOffer.coupon.targetPromotion.image.url
		local urlt3x = urlt3x:gsub('webp', 'png')
		local data = json.encode({url = urlt3x})
		image = true
		refresh_data("https://api.szprink.xyz/t3x/convert", data, {["api-version"] = "4.4", ["application-id"] = "%C5%BCappka", ["user-agent"] = "Synerise Android SDK 5.9.0 pl.zabka.apb2c", ["accept"] = "application/json", ["mobile-info"] = "horizon;28;AW700000000;9;CTR-001;nintendo;5.9.0", ["content-type"] = "application/json"}, "POST")
		local imageData = love.image.newImageData(love.filesystem.newFileData(imagebody, "image.t3x"))
		kuponimage = love.graphics.newImage(imageData)
	end
end
function png_acja()
	image = true
	if state == "main_strona" then
		if mojekupony.data.couponWallet.coupons[selectioncode + pagegap].__typename == "SingleCoupon" then
			local url = mojekupony.data.couponWallet.coupons[selectioncode + pagegap].singleCoupon.targetPromotion.image.url
			local url = url:gsub('webp', 'png')
			refresh_data(url, data, {}, "GET")
		elseif mojekupony.data.couponWallet.coupons[selectioncode + pagegap].__typename == "StackPloyCoupon" then
			local url = mojekupony.data.couponWallet.coupons[selectioncode + pagegap].ployCoupon.targetPromotion.image.url
			local url = url:gsub('webp', 'png')
			refresh_data(url, data, {}, "GET")
		end
	else
		local url = responded.data.ployOffer.ployOffer.coupon.targetPromotion.image.url
		local url = url:gsub('webp', 'png')
		refresh_data(url, data, {}, "GET")
	end
	local imageData = love.image.newImageData(love.filesystem.newFileData(imagebody, "image.png"))
	kuponimage = love.graphics.newImage(imageData)
end
function updatezappsy()
	--refresh_data("https://zabka-snrs.zabka.pl/schema-service/proxy/promotions?page=1&limit=20&type=CUSTOM&status=ASSIGNED%2CACTIVE&tagNames=kat_top&sort=priority%2Cdesc",  data, {["api-version"] = "4.4", ["authorization"] = "Bearer " .. authtoken, ["content-type"] = "application/json", ["accept"] = "application/json", ["user-agent"] = "okhttp/4.12.0"}, "GET")
	--topki = responded
	local data = json.encode({operationName = "LoyaltyPoints", query = "query LoyaltyPoints { loyaltyProgram { points pointsStatus pointsOperationsAvailable } }", variables = {}})
	local data = data:gsub('"variables":%[%]', '"variables":{}')
	refresh_data("https://api.spapp.zabka.pl/", data, {["user-agent"] = "Zappka/40038 (Horizon; nintendo/ctr; 56c41945-ba88-4543-a525-4e8f7d4a5812) REL/28", ["accept"] = "application/json", ["content-type"] = "application/json", ["authorization"] = "Bearer " .. authtoken}, "POST")
	if code == 200 then
		print("zappsy update success")
	end
end
function update_ployzones()
	local data = json.encode({operationName = "PloyZoneFeeds",variables = {},query = "query PloyZoneFeeds { ployZoneFeeds { zoneFeeds { id key icon { url } title } } }"})
	local data = data:gsub('"variables":%[%]', '"variables":{}')
	refresh_data("https://api.spapp.zabka.pl/", data, {["user-agent"] = "Zappka/40038 (Horizon; nintendo/ctr; 56c41945-ba88-4543-a525-4e8f7d4a5812) REL/28", ["accept"] = "application/json", ["content-type"] = "application/json", ["authorization"] = "Bearer " .. authtoken}, "POST")
	local ployzabka = responded.data.ployZoneFeeds.zoneFeeds[1].id
	local ploypartnerzy = responded.data.ployZoneFeeds.zoneFeeds[2].id
	local data = json.encode({operationName = "ZoneFeed", query = "query ZoneFeed($zoneId: ID!, $after: String) { zoneFeed(zoneFeedId: $zoneId, after: $after) { sections { __typename ... on OfferTileSection { __typename ...OfferTileSectionParts } ... on PromotionBannerCarousel { __typename ...PromotionBannerCarouselParts } ... on ProductPromotionListingCarousel { __typename ...ProductPromotionListingCarouselParts } ... on PloyOfferListingCarousel { __typename ...PloyOfferListingCarouselParts } ... on DealCarousel { __typename ...DealCarouselParts } ... on OptionTileSection { __typename ...OptionTileSectionParts } ... on StoriesSection { __typename ...StoriesSectionParts } } pagination { cursor } } }  fragment OfferTileSectionParts on OfferTileSection { id title tiles { id backgroundImage { url } offerTreeLevel { __typename ... on OfferCategory { id name } ... on PloyOfferListing { id title } ... on ProductPromotionListing { id title } } } }  fragment SwipeCarouselPropertiesParts on SwipeCarouselProperties { swipeId: id swipeTitle: title }  fragment AutoplayCarouselPropertiesParts on AutoplayCarouselProperties { autoplayId: id autoplayTitle: title autoplayIntervalMillis }  fragment PriceParts on Price { amount currencyCode fractionDigits }  fragment ColorParts on ThemeColor { light dark }  fragment BadgeParts on PromotionBadge { key label textColor { __typename ...ColorParts } }  fragment AbsoluteDiscountValueParts on AbsoluteDiscountValue { absoluteValue: value { __typename ...PriceParts } absoluteDiscount: discount { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment RelativeDiscountValueParts on RelativeDiscountValue { percentage relativeValue: value { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment MultibuyDiscountValueParts on MultibuyDiscountValue { multibuyValue: value { __typename ...PriceParts } maxQuantity triggerQuantity multibuyDiscount: discount { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } } }  fragment DiscountParts on DiscountValue { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } ... on MultibuyDiscountValue { __typename ...MultibuyDiscountValueParts } }  fragment HappyHourParts on HappyHour { alert validFrom validUntil }  fragment PromotionHighlightParts on PromotionHighlight { layout keyVisualImage { url } }  fragment TagParts on PromotionTag { backgroundColor { __typename ...ColorParts } key label longLabel textColor { __typename ...ColorParts } }  fragment ProductPromotionParts on ProductPromotion { id name image { url } detailsImage { url } contents detailsContents exclusivity isPricePerUnit hasLegalDetails hidePromotionAlerts legalShortDetails legalLongDetails validFrom validUntil promotionDurationDetails alternativeBasePrice { __typename ...PriceParts } basePrice { __typename ...PriceParts } badges { __typename ...BadgeParts } discount { __typename ...DiscountParts } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } tags { __typename ...TagParts } __typename }  fragment LargePromotionBannerPropertiesParts on LargePromotionBannerProperties { largeTitle: title largeSubtitle: subtitle image { url } }  fragment MediumPromotionBannerPropertiesParts on MediumPromotionBannerProperties { mediumTitle: title mediumSubtitle: subtitle image { url } }  fragment CtaParts on Cta { title subtitle action { label link } id image { url } __typename }  fragment DigitalProductPromotionParts on DigitalProductPromotion { id name code contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment PartnerProductPromotionParts on PartnerProductPromotion { id name contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment InAppProductPromotionParts on InAppProductPromotion { id name contents detailsContents happyHour { __typename ...HappyHourParts } productType image { url } validFrom validUntil }  fragment CouponParts on Coupon { id keyVisualImage { url } lastingAt validFrom validUntil showFullDate targetPromotion { __typename ... on ProductPromotion { __typename ...ProductPromotionParts id } ... on DigitalProductPromotion { __typename ...DigitalProductPromotionParts } ... on PartnerProductPromotion { __typename ...PartnerProductPromotionParts } ... on InAppProductPromotion { __typename ...InAppProductPromotionParts } } currentRedeemedQuantity possibleRedeems redeemLimitPerClient state couponSource __typename }  fragment DealParts on Deal { id coupon { __typename ...CouponParts id } ployPrice __typename }  fragment QuestParts on Quest { id title subtitle bannerImage { url } detailsImage { url } lastingAt description questCompleted repeatable badges progressSummary { status totalSteps finishedSteps } stages { status subStages { status description totalSteps finishedSteps } completionDescription } }  fragment PromotionBannerCarouselParts on PromotionBannerCarousel { id title carouselProperties: properties { __typename ... on SwipeCarouselProperties { __typename ...SwipeCarouselPropertiesParts } ... on AutoplayCarouselProperties { __typename ...AutoplayCarouselPropertiesParts } } banners { __typename ... on ProductPromotionBanner { id productPromotionListingRef { id } productPromotion { __typename ...ProductPromotionParts id } properties { __typename ... on LargePromotionBannerProperties { __typename ...LargePromotionBannerPropertiesParts } ... on MediumPromotionBannerProperties { __typename ...MediumPromotionBannerPropertiesParts } } } ... on CtaBanner { id cta { __typename ...CtaParts id } properties { __typename ... on LargePromotionBannerProperties { __typename ...LargePromotionBannerPropertiesParts } ... on MediumPromotionBannerProperties { __typename ...MediumPromotionBannerPropertiesParts } } header label } ... on Deal { __typename ...DealParts id } ... on LinkBanner { id header link properties { __typename ... on LargePromotionBannerProperties { __typename ...LargePromotionBannerPropertiesParts } ... on MediumPromotionBannerProperties { __typename ...MediumPromotionBannerPropertiesParts } } } ... on Quest { __typename ...QuestParts } } }  fragment ScrollCarouselPropertiesParts on ScrollCarouselProperties { id title showMore }  fragment ProductPromotionListingParts on ProductPromotionListing { id title productPromotions { __typename ...ProductPromotionParts id } pagination { cursor } }  fragment ProductPromotionListingCarouselParts on ProductPromotionListingCarousel { id title properties { __typename ...ScrollCarouselPropertiesParts } productPromotionListing { __typename ...ProductPromotionListingParts } }  fragment PloyOfferParts on PloyOffer { id coupon { __typename ...CouponParts id } price { base discounted } visualDiscount __typename }  fragment PloyOfferListingParts on PloyOfferListing { id title ployOffers { __typename ...PloyOfferParts id } pagination { cursor } }  fragment PloyOfferListingCarouselParts on PloyOfferListingCarousel { id properties { __typename ...ScrollCarouselPropertiesParts } ployOfferListing { __typename ...PloyOfferListingParts } }  fragment DealCarouselParts on DealCarousel { id deals { __typename ...DealParts id } properties { __typename ...ScrollCarouselPropertiesParts } }  fragment OptionTileParts on OptionTile { id label link }  fragment OptionTileSectionParts on OptionTileSection { id title subtitle optionTiles { __typename ...OptionTileParts } }  fragment StoriesSectionParts on StoriesSection { id title storiesSource labels }", variables = { zoneId = ployzabka}})
	refresh_data("https://api.spapp.zabka.pl/", data, {["user-agent"] = "Zappka/40038 (Horizon; nintendo/ctr; 56c41945-ba88-4543-a525-4e8f7d4a5812) REL/28", ["accept"] = "application/json", ["content-type"] = "application/json", ["authorization"] = "Bearer " .. authtoken}, "POST")
	zabkazonefeed = responded
	if zabkazonefeed ~= nil then
		print("updated ployzones!")
	end
	local data = json.encode({operationName = "ZoneFeed", query = "query ZoneFeed($zoneId: ID!, $after: String) { zoneFeed(zoneFeedId: $zoneId, after: $after) { sections { __typename ... on OfferTileSection { __typename ...OfferTileSectionParts } ... on PromotionBannerCarousel { __typename ...PromotionBannerCarouselParts } ... on ProductPromotionListingCarousel { __typename ...ProductPromotionListingCarouselParts } ... on PloyOfferListingCarousel { __typename ...PloyOfferListingCarouselParts } ... on DealCarousel { __typename ...DealCarouselParts } ... on OptionTileSection { __typename ...OptionTileSectionParts } ... on StoriesSection { __typename ...StoriesSectionParts } } pagination { cursor } } }  fragment OfferTileSectionParts on OfferTileSection { id title tiles { id backgroundImage { url } offerTreeLevel { __typename ... on OfferCategory { id name } ... on PloyOfferListing { id title } ... on ProductPromotionListing { id title } } } }  fragment SwipeCarouselPropertiesParts on SwipeCarouselProperties { swipeId: id swipeTitle: title }  fragment AutoplayCarouselPropertiesParts on AutoplayCarouselProperties { autoplayId: id autoplayTitle: title autoplayIntervalMillis }  fragment PriceParts on Price { amount currencyCode fractionDigits }  fragment ColorParts on ThemeColor { light dark }  fragment BadgeParts on PromotionBadge { key label textColor { __typename ...ColorParts } }  fragment AbsoluteDiscountValueParts on AbsoluteDiscountValue { absoluteValue: value { __typename ...PriceParts } absoluteDiscount: discount { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment RelativeDiscountValueParts on RelativeDiscountValue { percentage relativeValue: value { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment MultibuyDiscountValueParts on MultibuyDiscountValue { multibuyValue: value { __typename ...PriceParts } maxQuantity triggerQuantity multibuyDiscount: discount { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } } }  fragment DiscountParts on DiscountValue { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } ... on MultibuyDiscountValue { __typename ...MultibuyDiscountValueParts } }  fragment HappyHourParts on HappyHour { alert validFrom validUntil }  fragment PromotionHighlightParts on PromotionHighlight { layout keyVisualImage { url } }  fragment TagParts on PromotionTag { backgroundColor { __typename ...ColorParts } key label longLabel textColor { __typename ...ColorParts } }  fragment ProductPromotionParts on ProductPromotion { id name image { url } detailsImage { url } contents detailsContents exclusivity isPricePerUnit hasLegalDetails hidePromotionAlerts legalShortDetails legalLongDetails validFrom validUntil promotionDurationDetails alternativeBasePrice { __typename ...PriceParts } basePrice { __typename ...PriceParts } badges { __typename ...BadgeParts } discount { __typename ...DiscountParts } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } tags { __typename ...TagParts } __typename }  fragment LargePromotionBannerPropertiesParts on LargePromotionBannerProperties { largeTitle: title largeSubtitle: subtitle image { url } }  fragment MediumPromotionBannerPropertiesParts on MediumPromotionBannerProperties { mediumTitle: title mediumSubtitle: subtitle image { url } }  fragment CtaParts on Cta { title subtitle action { label link } id image { url } __typename }  fragment DigitalProductPromotionParts on DigitalProductPromotion { id name code contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment PartnerProductPromotionParts on PartnerProductPromotion { id name contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment InAppProductPromotionParts on InAppProductPromotion { id name contents detailsContents happyHour { __typename ...HappyHourParts } productType image { url } validFrom validUntil }  fragment CouponParts on Coupon { id keyVisualImage { url } lastingAt validFrom validUntil showFullDate targetPromotion { __typename ... on ProductPromotion { __typename ...ProductPromotionParts id } ... on DigitalProductPromotion { __typename ...DigitalProductPromotionParts } ... on PartnerProductPromotion { __typename ...PartnerProductPromotionParts } ... on InAppProductPromotion { __typename ...InAppProductPromotionParts } } currentRedeemedQuantity possibleRedeems redeemLimitPerClient state couponSource __typename }  fragment DealParts on Deal { id coupon { __typename ...CouponParts id } ployPrice __typename }  fragment QuestParts on Quest { id title subtitle bannerImage { url } detailsImage { url } lastingAt description questCompleted repeatable badges progressSummary { status totalSteps finishedSteps } stages { status subStages { status description totalSteps finishedSteps } completionDescription } }  fragment PromotionBannerCarouselParts on PromotionBannerCarousel { id title carouselProperties: properties { __typename ... on SwipeCarouselProperties { __typename ...SwipeCarouselPropertiesParts } ... on AutoplayCarouselProperties { __typename ...AutoplayCarouselPropertiesParts } } banners { __typename ... on ProductPromotionBanner { id productPromotionListingRef { id } productPromotion { __typename ...ProductPromotionParts id } properties { __typename ... on LargePromotionBannerProperties { __typename ...LargePromotionBannerPropertiesParts } ... on MediumPromotionBannerProperties { __typename ...MediumPromotionBannerPropertiesParts } } } ... on CtaBanner { id cta { __typename ...CtaParts id } properties { __typename ... on LargePromotionBannerProperties { __typename ...LargePromotionBannerPropertiesParts } ... on MediumPromotionBannerProperties { __typename ...MediumPromotionBannerPropertiesParts } } header label } ... on Deal { __typename ...DealParts id } ... on LinkBanner { id header link properties { __typename ... on LargePromotionBannerProperties { __typename ...LargePromotionBannerPropertiesParts } ... on MediumPromotionBannerProperties { __typename ...MediumPromotionBannerPropertiesParts } } } ... on Quest { __typename ...QuestParts } } }  fragment ScrollCarouselPropertiesParts on ScrollCarouselProperties { id title showMore }  fragment ProductPromotionListingParts on ProductPromotionListing { id title productPromotions { __typename ...ProductPromotionParts id } pagination { cursor } }  fragment ProductPromotionListingCarouselParts on ProductPromotionListingCarousel { id title properties { __typename ...ScrollCarouselPropertiesParts } productPromotionListing { __typename ...ProductPromotionListingParts } }  fragment PloyOfferParts on PloyOffer { id coupon { __typename ...CouponParts id } price { base discounted } visualDiscount __typename }  fragment PloyOfferListingParts on PloyOfferListing { id title ployOffers { __typename ...PloyOfferParts id } pagination { cursor } }  fragment PloyOfferListingCarouselParts on PloyOfferListingCarousel { id properties { __typename ...ScrollCarouselPropertiesParts } ployOfferListing { __typename ...PloyOfferListingParts } }  fragment DealCarouselParts on DealCarousel { id deals { __typename ...DealParts id } properties { __typename ...ScrollCarouselPropertiesParts } }  fragment OptionTileParts on OptionTile { id label link }  fragment OptionTileSectionParts on OptionTileSection { id title subtitle optionTiles { __typename ...OptionTileParts } }  fragment StoriesSectionParts on StoriesSection { id title storiesSource labels }", variables = { zoneId = ploypartnerzy}})
	refresh_data("https://api.spapp.zabka.pl/", data, {["user-agent"] = "Zappka/40038 (Horizon; nintendo/ctr; 56c41945-ba88-4543-a525-4e8f7d4a5812) REL/28", ["accept"] = "application/json", ["content-type"] = "application/json", ["authorization"] = "Bearer " .. authtoken}, "POST")
	patrnerzonefeed = responded
end
function updatepromki()
	if state == "category_sel" then
		currentpagename = currentpromkitable.data.offerCategoryFeed.offerCategory.levels[selectioncode + pagegap].title
	end
	if category ~= true then
		idshit = zabkazonefeed.data.zoneFeed.sections[1].tiles[selectioncode + pagegap].offerTreeLevel.id
	else
		idshit = currentpromkitable.data.offerCategoryFeed.offerCategory.levels[selectioncode + pagegap].id
	end
	local data = json.encode({operationName = "PloyOfferListing", query = "query PloyOfferListing($ployOfferListingId: ID!, $after: String) { ployOfferListing(ployOfferListingId: $ployOfferListingId, after: $after) { __typename ...PloyOfferListingParts } }  fragment PriceParts on Price { amount currencyCode fractionDigits }  fragment ColorParts on ThemeColor { light dark }  fragment BadgeParts on PromotionBadge { key label textColor { __typename ...ColorParts } }  fragment AbsoluteDiscountValueParts on AbsoluteDiscountValue { absoluteValue: value { __typename ...PriceParts } absoluteDiscount: discount { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment RelativeDiscountValueParts on RelativeDiscountValue { percentage relativeValue: value { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment MultibuyDiscountValueParts on MultibuyDiscountValue { multibuyValue: value { __typename ...PriceParts } maxQuantity triggerQuantity multibuyDiscount: discount { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } } }  fragment DiscountParts on DiscountValue { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } ... on MultibuyDiscountValue { __typename ...MultibuyDiscountValueParts } }  fragment HappyHourParts on HappyHour { alert validFrom validUntil }  fragment PromotionHighlightParts on PromotionHighlight { layout keyVisualImage { url } }  fragment TagParts on PromotionTag { backgroundColor { __typename ...ColorParts } key label longLabel textColor { __typename ...ColorParts } }  fragment ProductPromotionParts on ProductPromotion { id name image { url } detailsImage { url } contents detailsContents exclusivity isPricePerUnit hasLegalDetails hidePromotionAlerts legalShortDetails legalLongDetails validFrom validUntil promotionDurationDetails alternativeBasePrice { __typename ...PriceParts } basePrice { __typename ...PriceParts } badges { __typename ...BadgeParts } discount { __typename ...DiscountParts } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } tags { __typename ...TagParts } __typename }  fragment DigitalProductPromotionParts on DigitalProductPromotion { id name code contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment PartnerProductPromotionParts on PartnerProductPromotion { id name contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment InAppProductPromotionParts on InAppProductPromotion { id name contents detailsContents happyHour { __typename ...HappyHourParts } productType image { url } validFrom validUntil }  fragment CouponParts on Coupon { id keyVisualImage { url } lastingAt validFrom validUntil showFullDate targetPromotion { __typename ... on ProductPromotion { __typename ...ProductPromotionParts id } ... on DigitalProductPromotion { __typename ...DigitalProductPromotionParts } ... on PartnerProductPromotion { __typename ...PartnerProductPromotionParts } ... on InAppProductPromotion { __typename ...InAppProductPromotionParts } } currentRedeemedQuantity possibleRedeems redeemLimitPerClient state couponSource __typename }  fragment PloyOfferParts on PloyOffer { id coupon { __typename ...CouponParts id } price { base discounted } visualDiscount __typename }  fragment PloyOfferListingParts on PloyOfferListing { id title ployOffers { __typename ...PloyOfferParts id } pagination { cursor } }", variables = { ployOfferListingId = idshit}})
	refresh_data("https://api.spapp.zabka.pl/", data, {["user-agent"] = "Zappka/40038 (Horizon; nintendo/ctr; 56c41945-ba88-4543-a525-4e8f7d4a5812) REL/28", ["accept"] = "application/json", ["content-type"] = "application/json", ["authorization"] = "Bearer " .. authtoken}, "POST")
	if code ~= 404 then
		category = false
		currentpromkitable = responded
	else
		category = true 
		local data = json.encode({operationName = "CategoryFeed",query = "query CategoryFeed($categoryId: ID!, $after: String) { offerCategoryFeed(offerCategoryId: $categoryId, after: $after) { offerCategory { __typename ...OfferCategoryL1Parts } title sections { __typename ... on DealCarousel { __typename ...DealCarouselParts } ... on PloyOfferListingCarousel { __typename ...PloyOfferListingCarouselParts } ... on ProductPromotionListingCarousel { __typename ...ProductPromotionListingCarouselParts } ... on PromotionBannerCarousel { __typename ...PromotionBannerCarouselParts } ... on OptionTileSection { __typename ...OptionTileSectionParts } ... on StoriesSection { __typename ...StoriesSectionParts } } pagination { cursor } } }  fragment OfferCategoryL1Parts on OfferCategory { id name levels { __typename ... on OfferCategory { id name } ... on PloyOfferListing { id title } ... on ProductPromotionListing { id title } } }  fragment PriceParts on Price { amount currencyCode fractionDigits }  fragment ColorParts on ThemeColor { light dark }  fragment BadgeParts on PromotionBadge { key label textColor { __typename ...ColorParts } }  fragment AbsoluteDiscountValueParts on AbsoluteDiscountValue { absoluteValue: value { __typename ...PriceParts } absoluteDiscount: discount { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment RelativeDiscountValueParts on RelativeDiscountValue { percentage relativeValue: value { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment MultibuyDiscountValueParts on MultibuyDiscountValue { multibuyValue: value { __typename ...PriceParts } maxQuantity triggerQuantity multibuyDiscount: discount { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } } }  fragment DiscountParts on DiscountValue { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } ... on MultibuyDiscountValue { __typename ...MultibuyDiscountValueParts } }  fragment HappyHourParts on HappyHour { alert validFrom validUntil }  fragment PromotionHighlightParts on PromotionHighlight { layout keyVisualImage { url } }  fragment TagParts on PromotionTag { backgroundColor { __typename ...ColorParts } key label longLabel textColor { __typename ...ColorParts } }  fragment ProductPromotionParts on ProductPromotion { id name image { url } detailsImage { url } contents detailsContents exclusivity isPricePerUnit hasLegalDetails hidePromotionAlerts legalShortDetails legalLongDetails validFrom validUntil promotionDurationDetails alternativeBasePrice { __typename ...PriceParts } basePrice { __typename ...PriceParts } badges { __typename ...BadgeParts } discount { __typename ...DiscountParts } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } tags { __typename ...TagParts } __typename }  fragment DigitalProductPromotionParts on DigitalProductPromotion { id name code contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment PartnerProductPromotionParts on PartnerProductPromotion { id name contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment InAppProductPromotionParts on InAppProductPromotion { id name contents detailsContents happyHour { __typename ...HappyHourParts } productType image { url } validFrom validUntil }  fragment CouponParts on Coupon { id keyVisualImage { url } lastingAt validFrom validUntil showFullDate targetPromotion { __typename ... on ProductPromotion { __typename ...ProductPromotionParts id } ... on DigitalProductPromotion { __typename ...DigitalProductPromotionParts } ... on PartnerProductPromotion { __typename ...PartnerProductPromotionParts } ... on InAppProductPromotion { __typename ...InAppProductPromotionParts } } currentRedeemedQuantity possibleRedeems redeemLimitPerClient state couponSource __typename }  fragment DealParts on Deal { id coupon { __typename ...CouponParts id } ployPrice __typename }  fragment ScrollCarouselPropertiesParts on ScrollCarouselProperties { id title showMore }  fragment DealCarouselParts on DealCarousel { id deals { __typename ...DealParts id } properties { __typename ...ScrollCarouselPropertiesParts } }  fragment PloyOfferParts on PloyOffer { id coupon { __typename ...CouponParts id } price { base discounted } visualDiscount __typename }  fragment PloyOfferListingParts on PloyOfferListing { id title ployOffers { __typename ...PloyOfferParts id } pagination { cursor } }  fragment PloyOfferListingCarouselParts on PloyOfferListingCarousel { id properties { __typename ...ScrollCarouselPropertiesParts } ployOfferListing { __typename ...PloyOfferListingParts } }  fragment ProductPromotionListingParts on ProductPromotionListing { id title productPromotions { __typename ...ProductPromotionParts id } pagination { cursor } }  fragment ProductPromotionListingCarouselParts on ProductPromotionListingCarousel { id title properties { __typename ...ScrollCarouselPropertiesParts } productPromotionListing { __typename ...ProductPromotionListingParts } }  fragment SwipeCarouselPropertiesParts on SwipeCarouselProperties { swipeId: id swipeTitle: title }  fragment AutoplayCarouselPropertiesParts on AutoplayCarouselProperties { autoplayId: id autoplayTitle: title autoplayIntervalMillis }  fragment LargePromotionBannerPropertiesParts on LargePromotionBannerProperties { largeTitle: title largeSubtitle: subtitle image { url } }  fragment MediumPromotionBannerPropertiesParts on MediumPromotionBannerProperties { mediumTitle: title mediumSubtitle: subtitle image { url } }  fragment CtaParts on Cta { title subtitle action { label link } id image { url } __typename }  fragment QuestParts on Quest { id title subtitle bannerImage { url } detailsImage { url } lastingAt description questCompleted repeatable badges progressSummary { status totalSteps finishedSteps } stages { status subStages { status description totalSteps finishedSteps } completionDescription } }  fragment PromotionBannerCarouselParts on PromotionBannerCarousel { id title carouselProperties: properties { __typename ... on SwipeCarouselProperties { __typename ...SwipeCarouselPropertiesParts } ... on AutoplayCarouselProperties { __typename ...AutoplayCarouselPropertiesParts } } banners { __typename ... on ProductPromotionBanner { id productPromotionListingRef { id } productPromotion { __typename ...ProductPromotionParts id } properties { __typename ... on LargePromotionBannerProperties { __typename ...LargePromotionBannerPropertiesParts } ... on MediumPromotionBannerProperties { __typename ...MediumPromotionBannerPropertiesParts } } } ... on CtaBanner { id cta { __typename ...CtaParts id } properties { __typename ... on LargePromotionBannerProperties { __typename ...LargePromotionBannerPropertiesParts } ... on MediumPromotionBannerProperties { __typename ...MediumPromotionBannerPropertiesParts } } header label } ... on Deal { __typename ...DealParts id } ... on LinkBanner { id header link properties { __typename ... on LargePromotionBannerProperties { __typename ...LargePromotionBannerPropertiesParts } ... on MediumPromotionBannerProperties { __typename ...MediumPromotionBannerPropertiesParts } } } ... on Quest { __typename ...QuestParts } } }  fragment OptionTileParts on OptionTile { id label link }  fragment OptionTileSectionParts on OptionTileSection { id title subtitle optionTiles { __typename ...OptionTileParts } }  fragment StoriesSectionParts on StoriesSection { id title storiesSource labels }", variables = { categoryId = zabkazonefeed.data.zoneFeed.sections[1].tiles[selectioncode + pagegap].offerTreeLevel.id}})
		refresh_data("https://api.spapp.zabka.pl/", data, {["user-agent"] = "Zappka/40038 (Horizon; nintendo/ctr; 56c41945-ba88-4543-a525-4e8f7d4a5812) REL/28", ["accept"] = "application/json", ["content-type"] = "application/json", ["authorization"] = "Bearer " .. authtoken}, "POST")
		currentpromkitable = responded
	end
	if state ~= "category_sel" then
		if zabkazonefeed.data.zoneFeed.sections[1].tiles[selectioncode + pagegap].offerTreeLevel.title ~= nil then
			currentpagename = zabkazonefeed.data.zoneFeed.sections[1].tiles[selectioncode + pagegap].offerTreeLevel.title
		else
			currentpagename = zabkazonefeed.data.zoneFeed.sections[1].tiles[selectioncode + pagegap].offerTreeLevel.name
		end
	end
	if category ~= true then
		state = "promki"
	else 
		state = "category_sel"
	end
	selectioncode = 1
	pagegap = 0
end
function updatewallet()
	local data = json.encode({operationName = "CouponWallet",query = "query CouponWallet { couponWallet { total coupons { __typename ...SingleCouponParts ...StackPloyCouponParts ...StackDigitalCouponParts } } }  fragment PriceParts on Price { amount currencyCode fractionDigits }  fragment ColorParts on ThemeColor { light dark }  fragment BadgeParts on PromotionBadge { key label textColor { __typename ...ColorParts } }  fragment AbsoluteDiscountValueParts on AbsoluteDiscountValue { absoluteValue: value { __typename ...PriceParts } absoluteDiscount: discount { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment RelativeDiscountValueParts on RelativeDiscountValue { percentage relativeValue: value { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment MultibuyDiscountValueParts on MultibuyDiscountValue { multibuyValue: value { __typename ...PriceParts } maxQuantity triggerQuantity multibuyDiscount: discount { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } } }  fragment DiscountParts on DiscountValue { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } ... on MultibuyDiscountValue { __typename ...MultibuyDiscountValueParts } }  fragment HappyHourParts on HappyHour { alert validFrom validUntil }  fragment PromotionHighlightParts on PromotionHighlight { layout keyVisualImage { url } }  fragment TagParts on PromotionTag { backgroundColor { __typename ...ColorParts } key label longLabel textColor { __typename ...ColorParts } }  fragment ProductPromotionParts on ProductPromotion { id name image { url } detailsImage { url } contents detailsContents exclusivity isPricePerUnit hasLegalDetails hidePromotionAlerts legalShortDetails legalLongDetails validFrom validUntil promotionDurationDetails alternativeBasePrice { __typename ...PriceParts } basePrice { __typename ...PriceParts } badges { __typename ...BadgeParts } discount { __typename ...DiscountParts } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } tags { __typename ...TagParts } __typename }  fragment DigitalProductPromotionParts on DigitalProductPromotion { id name code contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment PartnerProductPromotionParts on PartnerProductPromotion { id name contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment InAppProductPromotionParts on InAppProductPromotion { id name contents detailsContents happyHour { __typename ...HappyHourParts } productType image { url } validFrom validUntil }  fragment CouponParts on Coupon { id keyVisualImage { url } lastingAt validFrom validUntil showFullDate targetPromotion { __typename ... on ProductPromotion { __typename ...ProductPromotionParts id } ... on DigitalProductPromotion { __typename ...DigitalProductPromotionParts } ... on PartnerProductPromotion { __typename ...PartnerProductPromotionParts } ... on InAppProductPromotion { __typename ...InAppProductPromotionParts } } currentRedeemedQuantity possibleRedeems redeemLimitPerClient state couponSource __typename }  fragment SingleCouponParts on SingleCoupon { singleCoupon { __typename ...CouponParts id } }  fragment StackPloyCouponParts on StackPloyCoupon { __typename ... on StackPloyCoupon { possibleRedeems ployCoupon { __typename ...CouponParts id } } }  fragment StackDigitalCouponParts on StackDigitalCoupon { __typename ... on StackDigitalCoupon { digitalCoupons { __typename ...CouponParts id } } }",variables = {}})
	local data = data:gsub('"variables":%[%]', '"variables":{}')
	refresh_data("https://api.spapp.zabka.pl/", data, {["user-agent"] = "Zappka/40038 (Horizon; nintendo/ctr; 56c41945-ba88-4543-a525-4e8f7d4a5812) REL/28", ["accept"] = "application/json", ["content-type"] = "application/json", ["authorization"] = "Bearer " .. authtoken}, "POST")
	mojekupony = responded
	if code == 200 then
		print("mam kupony hura")
	end
	selectioncode = 1
	pagegap = 0
end
function przelejen()
	changes = "przelej"
	love.keyboard.setTextInput(true, {type = "numpad", hint = "Numer Odbiorcy"})
	love.keyboard.setTextInput(false)
end
function tel_login()
	if love._potion_version == nil then
		if gui_design_mode == false then
			handle_authflow()
			numertel = "numertutaj"
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
function transfer2()
	changes = "pointsender"
	love.keyboard.setTextInput(true, {type = "numpad", hint = "Ilosc zappsow."})
	love.keyboard.setTextInput(false)
end
function transfer3()
	changes = "opissender"
	love.keyboard.setTextInput(true, {hint = "Opis przelewu."})
	love.keyboard.setTextInput(false)
end
function transfer4()
	changes = "recipe"
	love.keyboard.setTextInput(true, {hint = "Komu wysylasz."})
	love.keyboard.setTextInput(false)
end
function sendlol()
	senderime = name .. " (Żappka3DS User)"
	local data = json.encode({sender_name = senderime, recipient_name = recipename, points_number = tonumber(senderpoints), message = senderopis})
	refresh_data("https://api.zappka.app/transfer-points/users/" .. clientID, data, {["app-device"] = "samsung SM-A600FN", ["app-version"] = "3.21.60", ["api-version"] = "12", ["authorization"] = "Bearer " .. authtoken, ["content-type"] = "application/json", ["accept"] = "application/json", ["user-agent"] = "okhttp/4.12.0"}, "POST")
	print(body)
end
function transferenpointen(nrtel)
	local data = ""
	refresh_data("https://api.zappka.app/transfer-points/users/phone-number/" .. nrtel,  data, {["app-device"] = "samsung SM-A600FN", ["app-version"] = "3.21.60", ["api-version"] = "12", ["authorization"] = "Bearer " .. authtoken, ["content-type"] = "application/json", ["accept"] = "application/json", ["user-agent"] = "okhttp/4.12.0"}, "GET")
	print(body)
	clientID = responded.client_id
end
function test()
	local data = json.encode({idToken = boinaczejjebnie})
    refresh_data("https://www.googleapis.com/identitytoolkit/v3/relyingparty/getAccountInfo?key=AIzaSyDe2Fgxn_8HJ6NrtJtp69YqXwocutAoa9Q", data, {["Content-Type"] = "application/json", ["X-Android-Package"] = "pl.zabka.apb2c", ["X-Android-Cert"] = "FAB089D9E5B41002F29848FC8034A391EE177077", ["Accept-Language"] = "en-US", ["X-Client-Version"] = "Android/Fallback/X22003001/FirebaseCore-Android", ["X-Firebase-GMPID"] = "1:146100467293:android:0ec9b9022788ad32b7bfb4", ["X-Firebase-Client"] = "H4sIAAAAAAAAAKtWykhNLCpJSk0sKVayio7VUSpLLSrOzM9TslIyUqoFAFyivEQfAAAA", ["Content-Length"] = "894", ["User-Agent"] = "Dalvik/2.1.0 (Linux; U; Android 9; SM-A600FN Build/PPR1.180610.011)", ["Host"] = "www.googleapis.com", ["Connection"] = "Keep-Alive"}, "POST")
	
	
end
function handle_authflow()
	local data = json.encode({clientType = "CLIENT_TYPE_ANDROID"})
    refresh_data("https://www.googleapis.com/identitytoolkit/v3/relyingparty/signupNewUser?key=AIzaSyDe2Fgxn_8HJ6NrtJtp69YqXwocutAoa9Q", data, {["content-type"] = "application/json"}, "POST")
	boinaczejjebnie = responded.idToken
	
	
end
function sendvercode(nrtel)
	local data = json.encode({operationName = "SendVerificationCode", query = "mutation SendVerificationCode($input: SendVerificationCodeInput!) { sendVerificationCode(input: $input) { retryAfterSeconds } }",variables = {input = {phoneNumber = {countryCode = "48", nationalNumber = nrtel}}}})
	refresh_data("https://super-account.spapp.zabka.pl/", data, {["content-type"] = "application/json", ["authorization"] = responded.idToken}, "POST")
	
	
end
function sendbackvercode(smscode)  
	if gui_design_mode == false then
		local data = json.encode({operationName = "SignInWithPhone",variables = {input = {phoneNumber = {countryCode = "48", nationalNumber = numertel},verificationCode = smscode}}, query = "mutation SignInWithPhone($input: SignInInput!) { signIn(input: $input) { customToken } }"})
		refresh_data("https://super-account.spapp.zabka.pl/", data, {["content-type"] = "application/json", ["authorization"] = "Bearer " .. boinaczejjebnie, ["user-agent"] = "okhttp/4.12.0", ["x-apollo-operation-id"] = "a531998ec966db0951239efb91519560346cfecac77459fe3b85c5b786fa41de"	,["x-apollo-operation-name"] = "SignInWithPhone", ["accept"] = "multipart/mixed; deferSpec=20220824, application/json"}, "POST")
		
		
		local tokentemp = responded.data.signIn.customToken
		local data = json.encode({token = tokentemp, returnSecureToken = "true"})
		refresh_data("https://www.googleapis.com/identitytoolkit/v3/relyingparty/verifyCustomToken?key=AIzaSyDe2Fgxn_8HJ6NrtJtp69YqXwocutAoa9Q", data, {["content-type"] = "application/json"}, "POST")
		local refreshtemp = responded.refreshToken
		local tokentemp = responded.idToken
		local data = json.encode({idToken = tokentemp})
		refresh_data("https://www.googleapis.com/identitytoolkit/v3/relyingparty/getAccountInfo?key=AIzaSyDe2Fgxn_8HJ6NrtJtp69YqXwocutAoa9Q", data, {["content-type"] = "application/json"}, "POST")
		loadingtext = "Logowanie 45%..."
		uuidgen.seed()
		local data = json.encode({operationName = "SignIn",query = "mutation SignIn($signInInput: SignInInput!) { signIn(signInInput: $signInInput) { profile { __typename ...UserProfileParts } } }  fragment UserProfileParts on UserProfile { email gender }",variables = {signInInput = {sessionId = "65da013a-0d7d-3ad4-82bd-2bc15077d7f5"}}})
		refresh_data("https://api.spapp.zabka.pl/", data, {["user-agent"] = "Zappka/40038 (Horizon; nintendo/ctr; 56c41945-ba88-4543-a525-4e8f7d4a5812) REL/28", ["accept"] = "application/json", ["content-type"] = "application/json", ["authorization"] = "Bearer " .. tokentemp}, "POST")
		loadingtext = "Logowanie 65%..."
		authtoken = tokentemp
		print(authtoken)
		local data = json.encode({operationName = "QrCode", query ="query QrCode { qrCode { loyalSecret paySecret ployId } }", variables = {}})
		local data = data:gsub('"variables":%[%]', '"variables":{}')
		refresh_data("https://api.spapp.zabka.pl/", data, {["user-agent"] = "Zappka/40038 (Horizon; nintendo/ctr; 56c41945-ba88-4543-a525-4e8f7d4a5812) REL/28", ["accept"] = "application/json", ["content-type"] = "application/json", ["authorization"] = "Bearer " .. authtoken}, "POST")
		id = responded.data.qrCode.ployId
		love.filesystem.write("secret.hex.txt", responded.data.qrCode.loyalSecret)
		love.filesystem.write("id.txt", id)
		love.filesystem.write("token.txt", authtoken)
		local data = json.encode({operationName = "GetProfile", query = "query GetProfile { profile { id firstName birthDate phoneNumber { countryCode nationalNumber } email } }",variables = {}})
		local data = data:gsub('"variables":%[%]', '"variables":{}')
		refresh_data("https://super-account.spapp.zabka.pl/", data, {["user-agent"] = "Zappka/40038 (Horizon; nintendo/ctr; 56c41945-ba88-4543-a525-4e8f7d4a5812) REL/28", ["accept"] = "application/json", ["content-type"] = "application/json", ["authorization"] = "Bearer " .. tokentemp}, "POST")	
		name = responded.data.profile.firstName
		love.filesystem.write("imie.txt", name)
		love.filesystem.write("refresh.txt", refreshtemp)
		updatezappsy()
		zappsy = responded.data.loyaltyProgram.points
		calculatetotp()
	end
	state = "restartplz"
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
	elseif changes == "przelej" then
		transferenpointen(text)
		transfer2()
	elseif changes == "pointsender" then
		senderpoints = text
		transfer3()
	elseif changes == "opissender" then
		senderopis = text
		transfer4()
	elseif changes == "recipe" then
		recipename = text
		sendlol()
	end
end
function love.update(dt)
	if showredeemedtime <= 10 then
        showredeemedtime = showredeemedtime + (timerIncrement * dt)
    end
	if state == "main_strona" then
		if optiontable[3] == "true" then
			shiftRight(lookup)
		end
	end
    elapsedTime = elapsedTime + dt
	local time = love.timer.getTime()  
	if scrolltimerX <= 550 then
	   scrolltimerX = scrolltimerX + 0.2
	else
	   scrolltimerX = -150
	end
	
		
	
    elapsedTimeFade = elapsedTimeFade + dt

    
    if elapsedTime >= scrollDuration then
        isScrolling = false  
    end

    
    if elapsedTimeFade >= fadeDuration then
        isFading = false  
    end

    
    if stateChanged then
        elapsedTimeFade = 0
        isFading = true
        stateChanged = false
    end
    
    timer = timer - dt
	    
    love.graphics.origin()  
	collectgarbage("collect")
end