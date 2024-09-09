local channel = love.thread.getChannel("scroll")
local duration = channel:demand()  -- Get scroll duration from the main thread

local startY = 400
local endY = 210
local startFade = 1
local endFade = 0.5
local startbarY = 300
local endbarY = 20

local progress = 0
local easedProgress = 0
local fadeProgress = 0
local running = true

local function easeOutQuad(x)
    return 1 - (1 - x) ^ 2
end

while running do
    -- Receive elapsed time from the main thread
    local elapsedTime = channel:pop()

    if elapsedTime then
        -- Calculate progress of scrolling animation (0 to 1)
        progress = math.min(elapsedTime / duration, 1)
        easedProgress = easeOutQuad(progress)
        
        -- Calculate fade progress
        fadeProgress = easeOutQuad(math.min(elapsedTime / 0.5, 1))  -- Fade duration is 0.5

        -- Calculate new positions and fade levels
        local currentY = startY + (endY - startY) * easedProgress
        local barY = startbarY + (endbarY - startbarY) * easedProgress
        local currentFade = startFade + (endFade - startFade) * fadeProgress

        -- Send new values back to the main thread
        channel:push({currentY = currentY, barY = barY, currentFade = currentFade})

        -- Stop thread when progress is complete
        if progress >= 1 then
            running = false
        end
    end

    love.timer.sleep(0.01)  -- Sleep to allow other threads to execute
end
