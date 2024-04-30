
TextDraw = {}

function TextDraw.GetWrappedText(text, font, width, scale)
    scale = scale or 1
    text = tostring(text)
    local font = font or love.graphics.getFont()
    local newString = ""
    local line = ""
    for word in text:gmatch("%S+") do
        local testLine = line .. word .. " "
        local testWidth = TextDraw.GetTextWidth(testLine, font, scale)
        if testWidth > width then
            newString = newString .. line .. "\n"
            line = word .. " "
        else
            line = testLine
        end
    end
    newString = newString .. line
    return newString
end
function TextDraw.DrawText(text, x, y, color, font, scale)
    local font = font or love.graphics.getFont()
    local scale = scale or 1
    love.graphics.setFont(font)
    love.graphics.setColor(color)
    love.graphics.print(text, x, y, 0, scale, scale)
end


function TextDraw.DrawTextCentered(text, x, y, color, font, scale)
    local width = TextDraw.GetTextWidth(text, font, scale)
    local height = TextDraw.GetTextHeight(text, font, scale)
    TextDraw.DrawText(text, x - width / 2, y - height / 2, color, font, scale)
end


function TextDraw.GetTextWidth(text, font, scale)
    local font = font or love.graphics.getFont()
    local scale = scale or 1
    local width = font:getWidth(text)
    return width * scale
end

function TextDraw.GetTextHeight(text, font, scale)
    local font = font or love.graphics.getFont()
    local scale = scale or 1
    local height = font:getHeight(text)
    return height * scale
end

return TextDraw