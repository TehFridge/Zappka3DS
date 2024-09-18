local EasyHttps = {
    channelsChannel = love.thread.getChannel("channels")
}

local threadCode = [[
local https = require("https")
local channel = love.thread.getChannel("channels"):demand()

code, body, headers = https.request(...)

channel:push({code, body, headers})
]]

function EasyHttps.request(...)
    local thread = love.thread.newThread(threadCode)
    local channel = love.thread.newChannel()
    thread:start(...)
    EasyHttps.channelsChannel:supply(channel)
    local requestCoroutine = coroutine.create(function()
        while true do
            local msg = channel:pop()
            if msg ~= nil then 
                return unpack(msg)
            end
            coroutine.yield()
        end
    end)
    return requestCoroutine
end

function EasyHttps:syncAwait(request)
    while coroutine.status(request) ~= "dead" do
        coroutine.resume(request)
    end
end

local requestMemo = {}
setmetatable(requestMemo, {__mode = "kv"})

---Updates the request
---@param request thread Request thread
---@return boolean done true if completed
---@return number|nil code HTTPS status code
---@return string|nil body HTTPS Body
---@return table|nil headers HTTPS Headers
function EasyHttps:updateRequest(request)
    local s,c,b,h = coroutine.resume(request)
    if( not s and requestMemo[request]) then
        return true, unpack(requestMemo[request])
    end
    if(s and (c ~= nil)) then 
        requestMemo[request] = {c,b,h}
        return s,c,b,h
    end
    return false
end

return EasyHttps