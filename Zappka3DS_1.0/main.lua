require "lib.text-draw"
require("lib.bar128")
local Barcode = require("lib.bar128") -- an awesome Code128 library made by Nawias (POLSKA GUROM)
local font = love.graphics.newFont("bold.bcfnt") -- Font lol
local reference = 0
local scrolltimerX = -100
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
local endbarY = 100            -- Final Y position of the text
local isScrolling = false
local isFading = false
local currentY = startY
local banner = love.graphics.newImage("assets/banner.t3x")
local bottomdark = love.graphics.newImage("assets/frog.t3x")
local elapsedTimeFade = 0
local fadeDuration = 0.2
local stateChanged = false
local music = love.audio.newSource("bgm/bgm.ogg", "stream")
local sfx = love.audio.newSource("bgm/sfx.ogg", "static")
local sfx2 = love.audio.newSource("bgm/sfx2.ogg", "static")
name = "blank"
codeforinput = "blank"
existscode = love.filesystem.exists("kod.txt")
existsname = love.filesystem.exists("imie.txt")

function love.load()
    state = "main_strona"
	if existsname == false then -- Check whether the save file with the name exists or nah
        name = "3DS"
	else 
	    name =  love.filesystem.read("imie.txt")
	end
	
    if existscode == false then -- Check whether the save file with the barcode exists or nah
        codeforinput = "101010101010"
	  else 
	    codeforinput =  love.filesystem.read("kod.txt")
    end
    barcode = Barcode(codeforinput, 60, 3)
	music:setLooping(true)
    music:play()
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
		barcode:draw('notext', 17, barY)
		love.graphics.setColor(0.27,0.84,0.43,currentFade)
        love.graphics.print('Cześć, ' .. name, font, 10, 10, 0, 3, 3)
		TextDraw.DrawTextCentered(codeforinput, SCREEN_WIDTH/2, currentY - 25, {0,0,0,1}, font, 1.9)
		TextDraw.DrawTextCentered("Zeskanuj swój Kod Kreskowy", SCREEN_WIDTH/2, currentY, {0.27,0.84,0.43,1}, font, 2.3)
    end
end

function draw_bottom_screen()
    SCREEN_WIDTH = 400
    SCREEN_HEIGHT = 240
    love.graphics.setColor(1, 1, 1, 1)
    love.graphics.draw(bottomdark, 0, 0)
    love.graphics.setColor(0, 0, 0, 1)
    love.graphics.print("A - Zobacz swój kod", font, 20, 10, 0, 1.2)
	if state == "barcode" then
		love.graphics.print("Y - Zmień swój kod", font, 20, 25, 0, 1.2)
	end
	love.graphics.print("X - Zmień swoją nazwe", font, 20, 40, 0, 1.2)
    love.graphics.setColor(1, 1, 1, 1)
       
end

function love.gamepadpressed(joystick, button)
    if button == "a" then
        if state == "barcode" then
		    sfx2:play()
            state = "main_strona"
			isFading = false
        else
		    sfx:play()
            state = "barcode"
			isScrolling = true
			isFading = true
            elapsedTime = 0  -- Reset elapsed time for scrolling animation
			elapsedTimeFade = 0
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
	
    love.graphics.origin()  
end