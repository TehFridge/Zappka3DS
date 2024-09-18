--[[
	Lavis Minimal Version
	Author: Neer

	No globals, no dependencies! Not bs! Only ButtonS (image buttons) and
	the general widget functions
]]

local LIB_PATH=(...)
local eF=function() end --empty function

local lavis={
	directDraw,   --if Lavis shouldn't draw to the canvas? nil by default
	needsRefresh, --should one redraw the stuff to the canvas?
	canvas,       --lavis will draw things to the canvas which makes it efficient!!
	cache={},     --lavis can load images from file! so it maintains a cache!
	imgui,        --lavis is not just retained!! It also supports immediate mode
	canvas,       --for best performance optimization Lavis uses a canvas!!
	widgets={}    --note widget is a class and widgets is a collection of its objects
}


--This takes care of (perhaps) all canvas problems!
lavis.canvas=love.graphics.newCanvas(love.graphics.getDimensions())
local setMode=love.window.setMode
lavis.resize=function(w,h)
	lavis.canvas=love.graphics.newCanvas(w,h)
end
love.window.setMode=function(w,h,...)
	setMode(w,h,...)
	lavis.resize(w,h)
end

function lavis.refresh() lavis.needsRefresh=true end

function lavis.newImage(path)
	if lavis.cache[path] then return lavis.cache[path] end
	local img=love.graphics.newImage(path)
	lavis.cache[path]=img
	return img
end

--In the minimal version we only take care of mouse input!!!
--also the callback functions are limited for the minimal version
local imgui={
	drawBorder,drawOrigin;        --if imgui is in debug mode
	entered={};   --table to take care of mouse entered which widget!
	pressed={};   --table to take care of mouse pressed which widget!
	mx,my;                --the position of the mouse!
	clicked;              --the button that was clicked (nil if not clicked)
	clickType;            --if button was 'pressed' or 'released'
	whileHovered=eF; --what to do while mouse is hovered over widget
	whilePressed=eF; --what to do while a widget is pressed
	onMouseEnter=eF;      --what to do when mouse enters a widget
	onMouseExit=eF;       --what to do when mouse exits a widget
	onClick=eF;    --what to do when a widget is pressed
	onRelease=eF;   --what to do when a widget is released
}

lavis.imgui=imgui --we want the user to be able to access this!!
--[[
	Lavis ImGUI functions are seperated from retained functions
	in case anyone wants to use only the ImGUI functionaliy!
--]]
local btnDown=love.mouse.isDown
function lavis.imgui.update(dt)
	if imgui.clickType=='pressed' then
		imgui.clicked=imgui.clicked and btnDown(imgui.clicked) and imgui.clicked
	elseif imgui.clickType=='released' then
		imgui.clickType=nil
	else
		imgui.clicked=nil
	end
	imgui.mx,imgui.my=love.mouse.getPosition()
end

function lavis.imgui.mousepressed(x,y,btn)
	imgui.clicked,imgui.clickType=btn,'pressed'
end

function lavis.imgui.mousereleased(x,y,btn)
	imgui.clicked,imgui.clickType=btn,'released'
end

function lavis.drawImageButton(id,img,x,y,w,h,ox,oy,r)
	local sx,sy
	if type(id)~='number' then
		img,x,y,w,h,ox,oy,r,id=id,img,x,y,w,h,ox,oy
	end
	ox,oy=ox or 0,oy or 0
	img=type(img)~='string' and img or lavis.newImage(img)
	if not w then
		w,h=img:getDimensions()
		sx,sy=1,1
	else
		sx,sy=w/img:getWidth(),h/img:getHeight()
	end
	local draw=function()
		love.graphics.draw(img,x,y,r or 0,sx,sy,ox,oy)
		imgui.debugDraw(x,y,w,h,ox,oy)
	end
	lavis.imgui.render(draw,id,x-ox,y-oy,w,h)
end

--Helper functions for imgui rendering!!

local function renderArrow(x,y,dir,a,b,c)
	
	a,b,c=a or 20,b or 5,c or 30

	if dir=='right' or dir=='left' then
		if dir=='left' then a,c=-a,-c end
		love.graphics.line(x,y,x+a,y)
		love.graphics.polygon('fill',
			x+a,y-b,
			x+a,y+b,
			x+c,y
		)
	elseif dir=='down' or dir=='up' then
		if dir=='up' then a,c=-a,-c end
		love.graphics.line(x,y,x,y+a)
		love.graphics.polygon('fill',
			x-b,y+a,
			x+b,y+a,
			x,y+c
		)
	end
end

function lavis.imgui.debugDraw(x,y,w,h,ox,oy)
	if imgui.drawBorder then
		love.graphics.setColor(0,1,0,1)
		love.graphics.rectangle('line',x-ox,y-oy,w,h)
	end
	if imgui.drawOrigin then
		love.graphics.setColor(0,1,0,0.7)
		love.graphics.circle('fill',x,y,5)
		if ox<w then renderArrow(x,y,"right") end
		if ox>0 then renderArrow(x,y,"left") end
		if oy<h then renderArrow(x,y,"down") end
		if oy>0 then renderArrow(x,y,"up") end
	end
end

function lavis.imgui.render(func,id,x,y,w,h)
	if not id or id<1 then id=1 end
	local r,g,b,a=love.graphics.getColor()
	if lavis.aabb(x,y,w,h,imgui.mx,imgui.my) then
		imgui.whileHovered(id,x,y,w,h)
		if not imgui.entered[id] then
			imgui.onMouseEnter(id,x,y,w,h)
			imgui.entered[id]=true
		end
		if imgui.clicked then
			imgui.whilePressed(id,imgui.clicked,x,y,w,h)
			if imgui.clickType=='pressed' then
				if not imgui.pressed[id] then
					imgui.onClick(id,imgui.clicked,x,y,w,h)
					imgui.pressed[id]=true
				end
			else
				if imgui.pressed[id] then
					imgui.onRelease(id,imgui.clicked,x,y,w,h)
					imgui.pressed[id]=nil
				end
			end
		end
	else
		if imgui.entered[id] then
			imgui.onMouseExit(id,x,y,w,h)
			imgui.entered[id]=nil
		end
	end
	func()
	love.graphics.setColor(r,g,b,a)
end


--[[
	Just defines some common functions like aabb collision,etc.
	For users using push you might want to override getCursorPosition
--]]

function lavis.aabb(x1,y1,w1,h1, x2,y2,w2,h2)
	w2,h2=w2 or 0,h2 or 0
	return x1 < x2+w2 and x2 < x1+w1 and y1 < y2+h2 and y2 < y1+h1
end

--if something is within bounds of an ellipse
function lavis.pie(x,y,a,b, x0,y0)
	return lavis.aabb(x-a,y-b,2*a,2*b, x0,y0) and 
		(((x-x0)*(x-x0)/(a*a)) + ((y-y0)*(y-y0)/(b*b))<=1)
end

function lavis.getCursorX() return love.mouse.getX() end
function lavis.getCursorY() return love.mouse.getY() end

function lavis.getCursorPosition() return love.mouse.getPosition() end


--[[
	Every widget share some properties like position, size and some events.
	So every widget rather then redefining these common properties, will
	simply inherit from the Widget class.
]]

lavis.widget={
	id=nil,                    --the id of the widget
	name=nil,                  --a property you could use to identify the widget
	visible=true,              --is the widget visible?
	enabled=true,              --whether you can interact with the widget
	value=nil,                 --extra information for each widget
	image=nil,                 --assumption: most widgets *will* have an image
	entered=false,             --state of the widget (hovered or not)
	dragging=nil,              --the button that's being used to drag the widget
	focused=false,             --if you click on a widget once it's focused
	x=0,                       --x and y for position of the widget
	y=0,
	imgX=0,                    --imgX and imgY for position of the image
	imgY=0,
	r=0,                       --r for rotating the image
	sx=1,                      --sx,sy for scaling the image
	sy=1,
	ox=0,                      --ox,oy for setting origin of the image
	oy=0,
	drawColor=nil,             --an array in {r,g,b,a} format, sets color before drawing
	drawBorder=nil,            --if true draws a simple border around the widget (UFD)
	drawOrigin=nil,            --if true draws the origin of the widget and image(UFD)
	shape='box',               --useful for detecting hover (box,circle,ellipse)
	width=nil,                 --width and height only useful for box shape
	height=nil,
	radiusa=nil,               --for circle and ellipse
	radiusb=nil,               --for ellipse only
	responsive=false,           --whether changing widget's size affects image's size
	frozen=nil,                --if frozen is true then *widget* won't move or scale
	escapebtns=nil,            --the btns which on pressing makes the widget lose focus
	hotbtns=nil,            --the mouse btns which on pressing makes the widget gain focus

	onRelease=eF,      --when a MB is released over the widget
	onClick=eF,        --when a MB is clicked over the widget
	onDrag=eF,         --when a MB is clicked over the widget to drag it
	onResize=eF,       --when the widget is resized!
	onMove=eF,         --when the widget is moved!
	onKeyPress=eF,     --when a certain key is pressed *while focused*
	onKeyRelease=eF,   --when a certain key is released *while focused*
	onFocusGained=eF,  --called when the widget is focused
	onFocusLost=eF,    --called when the widget has lost focus
	whileFocused=eF,   --called as long as the widget is focused
	whileKeyPressed=eF,--*as long as* a key is pressed *while* focused
	whilePressed=eF,   --is called *while* LMB is down
	whileHovered=eF,   --is called *while* the cursor is over the widget
	onMouseMove=eF,    --is called when the cursor moves while still hovered
	onWheelMove=eF,    --is called when scrolled while the widget is still focused
	onMouseEnter=eF,   --when cursor enters the widget area
	onMouseExit=eF,    --when cursor exits the widget area
	onValueChange=eF,  --when the value of the widget is changed
	onDraw=eF,         --called after the widget is drawn
}

local r,g,b,a  -- to get the default color

function lavis.widget:init(...) self:initWidget(...) end

function lavis.widget:initWidget(x,y,shape,...)
	self.escapebtns,self.hotbtns={true,true},{true}
	assert(type(shape)=='string',
		"Lavis Error! Expected a string for shape, got '"..shape.."' !"
	)
	self:setShape(shape,...)
	self.id=#lavis.widgets+1
	table.insert(lavis.widgets,self)
	self.responsive=true
	self:setPosition(x,y)
end

function lavis.widget:setShape(shape,...)
	self.shape=shape
	self:setSize(...)
end

function lavis.widget:setPosition(x,y,ignore)
	if self.frozen then return end
	lavis.refresh()
	self.x=x or self.x
	self.y=y or self.y
	if self.responsive and not ignore then self:setImagePosition(x,y) end
	self:onMove(x,y,ignore)
end

function lavis.widget:getSize()
	if self.shape=='box' then return self.width,self.height
	elseif self.shape=='circle' then return self.radiusa
	elseif self.shape=='ellipse' then return self.radiusa,self.radiusb end
end

function lavis.widget:setSize(arg1,arg2,arg3)

	if self.frozen then return end
	lavis.refresh()
	local origw,origh=self.width,self.height

	if self.shape=='box' then
		if not self.width then
			self.width,self.height=arg1,arg2
			return
		end		
		--arg1,arg2 are w,h and arg3 is relative

		-- useful information for translating the widget responsively
		local dx,dy=self.width,self.height

		--in case the box is a square
		if type(arg2)=='boolean' then arg3=arg2 end
		arg2=type(arg2)=='number' and arg2 or arg1

		if arg3==true then
			dx,dy=arg1,arg2
			arg1,arg2=self.width+arg1,self.height+arg2
		else
			dx=arg1-self.width
			dy=arg2-self.height
		end

		self.width,self.height=arg1,arg2
		
		if self.responsive and self.image then
			self.x=self.x-dx/(self.image:getWidth()/self.ox)
			self.y=self.y-dy/(self.image:getHeight()/self.oy)
			self:setImageSize(arg1,arg2,false)
		end

	elseif self.shape=='circle' then
		--arg1,arg2 are radius,relative and arg3 is nil
		if arg2==true then
			self.radiusa=(self.radiusa or 0)+arg1
		else
			self.radiusa=arg1
		end
		if self.responsive then
			self:setImageSize(arg1*2,arg1*2,arg2)
		end
	elseif self.shape=='ellipse' then
		assert(arg1 and arg2,"lavis Error! You must provide both the radiuses!")
		if arg3 then arg1,arg2=self.radiusa+arg1,self.radiusb+arg2 end
		self.radiusa,self.radiusb=arg1,arg2
		if self.responsive then
			self:setImageSize(arg1*2,arg2*2,false)
		end
	else
		error("lavis Error! The given string is not a valid shape!")
	end
	self:onResize(arg1,arg2,arg3)
end

function lavis.widget:updateSize()
	if not self.responsive then return end
	if self.shape=='circle' then
		self:setSize(math.max(self:getImageSize()))
	elseif self.shape=='ellipse' then
		self:setSize(self:getImageWidth()/2,self:getImageHeight()/2)
	else
		self:setSize(self:getImageSize())
	end
	
end

function lavis.widget:getColor() return unpack(self.drawColor) end
function lavis.widget:setColor(...) lavis.refresh() self.drawColor={...} end
function lavis.widget:getOpacity() return self.drawColor[4] end
function lavis.widget:setOpacity(a)
	lavis.refresh()
	self.drawColor=self.drawColor or {1,1,1} self.drawColor[4]=a
end

function lavis.widget:setImage(url)
	lavis.refresh()
	self.image=type(url)~='string' and url or lavis.newImage(url)
end

--TODO: Remove self.ox,self.oy from this!
function lavis.widget:setImagePosition(x,y)
	lavis.refresh()
	self.imgX = x or self.imgX
	self.imgY =y or self.imgY
end

function lavis.widget:getImagePosition()
	return self.imgX,self.imgY
end

function lavis.widget:getImageWidth()
	return self.image and self.sx*self.image:getWidth() or 0
end
function lavis.widget:getImageHeight()
	return self.image and self.sy*self.image:getHeight() or 0
end
function lavis.widget:getActualImageSize() return self.image:getDimensions() end
function lavis.widget:getImageSize()
	return self:getImageWidth(),self:getImageHeight()
end

function lavis.widget:setImageSize(w,h,relative)

	if not self.image then return end
	lavis.refresh()
	if type(h)=='boolean' or not h then h,relative=w,h end	--for square
	if relative then w,h=w+self:getImageWidth(),h+self:getImageHeight() end

	self.sx,self.sy=w/self.image:getWidth(),h/self.image:getHeight()
end

function lavis.widget:setImageRotation(r) lavis.refresh() self.r=r end
function lavis.widget:setImageOrigin(...)
	self.ox,self.oy=...
	if self.shape=='box' then
		self:setImagePosition(self.imgX+self.ox,self.imgY+self.oy)
	end
end

-- function lavis.widget:showOrigin() self.drawOrigin=true end
-- function lavis.widget:hideOrigin() self.drawOrigin=nil end

-- function lavis.widget:showBorder() self.drawBorder=true end
-- function lavis.widget:hideBorder() self.drawBorder=nil end

function lavis.widget:isFrozen() return self.frozen end
-- function lavis.widget:freeze() self.frozen=true end
-- function lavis.widget:unfreeze() self.frozen=nil end

function lavis.widget:setFrozen(val)  self.frozen=val end
function lavis.widget:setVisible(val) self.visible=val end
function lavis.widget:toggleVisibilty() self.visible=not self.visible end
function lavis.widget:setEnabled(val) self.enabled=val end
function lavis.widget:setName(val) self.name=val end
function lavis.widget:setValue(val)
	local prev=self.value
	self.value=val
	self:onValueChange(val,prev)
end
function lavis.widget:setWireframe(val) lavis.refresh() self.drawBorder,self.drawOrigin=val,val end
function lavis.widget:setResponsive(val) self.responsive=val end
function lavis.widget:isResponsive(val) return self.responsive end

function lavis.widget:setValue(val) self.value=val end
function lavis.widget:getValue() return self.value end

function lavis.widget:setDepth(z)
	z=math.min(math.max(z,1),#lavis.widgets)
	lavis.widgets[z],lavis.widgets[self.id]=
	lavis.widgets[self.id],lavis.widgets[z]
	lavis.refresh()
end

function lavis.widget:setFocus(val,...)
	if val==true then
		if not self.focused then
			self.focused=true
			self:onFocusGained(...)
		end
	else
		if self.focused then
			self.focused=false
			self:onFocusLost(...)
		end
	end
end

lavis.widget.setFocused=lavis.widget.setFocus

function lavis.widget:isEnabled() return self.enabled end
-- function lavis.widget:enable() self.enabled=true end
-- function lavis.widget:disable() self.enabled=false end

function lavis.widget:isVisible() return self.visible end
-- function lavis.widget:show() self.visible=true end
-- function lavis.widget:hide() self.visible=false end


function lavis.widget:update(dt)
	if self.focused then self.whileFocused() end
	if self:isHovered() then
		self.whileHovered()
		if not self.entered then
			self.entered=true
			self:onMouseEnter()
		end
		if love.mouse.isDown(1) then
			self.whilePressed()
		end
	else
		if self.entered then
			self.entered=false
			self:onMouseExit()
		end
	end
end

function lavis.widget:keypressed(key,...)
	if self.focused then self:onKeyPress(key,...) end
end

function lavis.widget:keyreleased(key,...)
	if self.focused then self:onKeyRelease(key,...) end
end

function lavis.widget:mousepressed(x,y,button,...)
	self.dragging=button
	if self:isHovered(x,y) then
		if self.hotbtns[button] then
			self:setFocus(true,button,x,y,...)
		end
		self:onClick(button,x,y,...)
	else
		if self.escapebtns[button] then
			self:setFocus(false,button,x,y,...)
		end
	end
end

function lavis.widget:mousemoved(...)
	if self:isFocused() then
		if self.dragging then self.onDrag(self.dragging,...) end
		self:onMouseMove(...)
	end
end

function lavis.widget:wheelmoved(x,y)
	if self.focused then
		self:onWheelMove(x,y)
	end
end

function lavis.widget:mousereleased(x,y,button,...)
	self.dragging=nil
	if self.focused then
		self:onRelease(button,x,y,...)
	end
end

function lavis.widget:isHovered(mx,my)
	if not mx then mx,my=lavis.getCursorPosition() end

	if self.shape=='box' then
		return lavis.aabb(self.x,self.y,self.width,self.height,mx,my)

	elseif self.shape=='circle' then
		return lavis.pie(self.x,self.y,self.radiusa,self.radiusa,mx,my)

	elseif self.shape=='ellipse' then
		return lavis.pie(self.x,self.y,self.radiusa,self.radiusb,mx,my)
	end
end

function lavis.widget:isFocused() return self.focused end

function lavis.widget:setEscapeButton(btn,remove)
	self.escapebtns[key]=not remove
end

function lavis.widget:removeEscapeButtons()
	self.escapebtns={}
end

function lavis.widget:setHotButton(btn,remove)
	self.hotbtns[btn]=not remove
end

function lavis.widget:connectSignal(signal, handler)
	if signal=="click" or "hit" then
		self.onClick=handler
	elseif signal=="drag" then
		self.onDrag=handler
	elseif signal=="release" then
		self.onRelease=handler
	elseif signal=="hovered" then
		self.whileHovered=handler
	elseif signal=="pressed" or signal=="clicked" then
		self.whilePressed=handler
	elseif signal=="mousemoved" then
		self.onMouseMove=handler
	elseif signal=="wheelmoved" or signal=="scroll" then
		self.onWheelMove=handler
	elseif signal=="focused" then
		self.whileFocused=handler
	elseif signal=="focusgained" or signal=="focus" then
		self.onFocusGained=handler
	elseif signal=="focuslost" or signal=="blur" then
		self.onFocusLost=handler
	elseif signal=="mouseentered" then
		self.onMouseEnter=handler
	elseif signal=="mouseexited" then
		self.onMouseExit=handler
	elseif signal=="move" then
		self.onMove=handler
	elseif signal=="draw" then
		self.onDraw=handler
	elseif signal=="resize" then
		self.onResize=handler
	elseif signal=="valuechanged" then
		self.onValueChange=handler
	end
	return self
end

lavis.widget.addEventListener=lavis.widget.connectSignal

function lavis.widget:render(toCanvas)
	if self.drawColor then
		r,g,b,a=love.graphics.getColor()
		love.graphics.setColor(unpack(self.drawColor))
	end

	love.graphics.draw(self.image,self.imgX,self.imgY,
		self.r,self.sx,self.sy,self.ox,self.oy)
	self:onDraw()
	
	if self.drawBorder then self:renderBorder() end
	if self.drawOrigin then self:renderOrigin() end
	
	if self.drawColor then love.graphics.setColor(r,g,b,a) end
end

function lavis.widget:renderOrigin()

	love.graphics.setColor(1,0,0,1)
	love.graphics.circle('fill',self.x,self.y,5)
	
	renderArrow(self.x,self.y,"right")
	renderArrow(self.x,self.y,"down")
	if self.shape~='box' then
		renderArrow(self.x,self.y,"up")
		renderArrow(self.x,self.y,"left")
	end
	
	if self.image then
		love.graphics.setColor(0,1,0,0.7)
		love.graphics.circle('fill',self.imgX,self.imgY,5)
	
		if self.ox<self.image:getWidth() then
			renderArrow(self.imgX,self.imgY,"right")
		end
		if self.ox>0 then
			renderArrow(self.imgX,self.imgY,"left")
		end
		if self.oy<self.image:getHeight() then
			renderArrow(self.imgX,self.imgY,"down")
		end
		if self.oy>0 then
			renderArrow(self.imgX,self.imgY,"up")
		end
	end
	love.graphics.setColor(1,1,1)
end

function lavis.widget:renderBorder()
	if self.focused then
		love.graphics.setColor(1,0,0,1)
	else
		love.graphics.setColor(0,1,0,1)
	end
	if self.shape=='box' then
		love.graphics.rectangle('line',self.x,self.y,self.width,self.height)
	elseif self.shape=='circle' then
		love.graphics.circle('line',self.x,self.y,self.radiusa)
		love.graphics.rectangle('line',self.x-self.radiusa,self.y-self.radiusa,self.radiusa*2,self.radiusa*2)
	elseif self.shape=='ellipse' then
		love.graphics.rectangle('line',self.x-self.radiusa,self.y-self.radiusb,self.radiusa*2,self.radiusb*2)
		love.graphics.ellipse('line',self.x,self.y,self.radiusa,self.radiusb)
	end
	love.graphics.setColor(1,1,1)
end

function lavis.widget:destroy()
	lavis.removeWidget(self)
end

setmetatable(lavis.widget, {__call = function(c, ...)
	local o = setmetatable({}, c)
	o:init(...)
	return o
end})

-- borrowed from itable library
local function tablecopy(srcTbl)
	local destTbl={}
	for i in pairs(srcTbl) do
		if type(srcTbl[i])~='table' then
			destTbl[i]=srcTbl[i]
		else
			destTbl[i]=tablecopy(srcTbl[i])
		end
	end
	return destTbl
end
lavis.renderImageButton=lavis.drawImageButton
lavis.widget.draw=lavis.widget.render

lavis.imageButton=tablecopy(lavis.widget)

function lavis.imageButton:init(url,x,y,...)
	self:setImage(url)
	x,y=x or 0,y or 0
	if (...) then
		self:initWidget(x,y,...)
	else
		self:initWidget(x,y,"box",self.image:getDimensions())
	end		
end


setmetatable(lavis.imageButton, {__call = function(c, ...)
	local o = setmetatable(tablecopy(lavis.widget), c)
	o.init=c.init
	o:init(...)
	return o
end})


function forEachWidgetC(func,cond,...)
	for _,widget in ipairs(lavis.widgets) do
		if widget[cond] then
			widget[func](widget,...)
		end
	end
end
function lavis.forEachWidget(func)
	for _,widget in ipairs(lavis.widgets) do func(widget) end
end

function lavis.getWidget(id)
	return lavis.widgets[id]
end

function lavis.getWidgetByName(name)
	for i,widget in ipairs(lavis.widgets) do
		if widget.name==name then
			return widget,i
		end
	end
end

function lavis.removeWidget(widget)
	widget=type(widget)=='table' and widget.id or select(2,lavis.getWidgetByName(widget))
	for i=widget,#lavis.widgets do
		lavis.widgets[i].id=lavis.widgets[i].id-1
	end
	table.remove(lavis.widgets,widget)
end

lavis.remove=lavis.removeWidget

function lavis.mousepressed(...) imgui.mousepressed(...)  forEachWidgetC('mousepressed','enabled',...) end
function lavis.keypressed(...) forEachWidgetC('keypressed','enabled',...) end
function lavis.keyreleased(...) forEachWidgetC('keyreleased','enabled',...) end
function lavis.mousereleased(...) imgui.mousereleased(...) forEachWidgetC('mousereleased','enabled',...) end
function lavis.mousemoved(...) forEachWidgetC('mousemoved','enabled',...) end
function lavis.wheelmoved(...) forEachWidgetC('wheelmoved','enabled',...) end
function lavis.update(dt) imgui.update(dt) forEachWidgetC('update','enabled',dt) end
function lavis.draw(...)
	if lavis.directDraw then
		return forEachWidgetC('render','visible')
	end
	if lavis.needsRefresh then
		love.graphics.setCanvas(lavis.canvas)
		love.graphics.clear()
		forEachWidgetC('render','visible')
		love.graphics.setCanvas()
		lavis.needsRefresh=false
	end
	--try your best to do 
	love.graphics.draw(lavis.canvas,...)
end
function lavis.setWireframe(val)
	imgui.drawBorder,imgui.drawOrigin=val,val
	lavis.forEachWidget(function(w) w:setWireframe(val) end)
end
function lavis.enableAll() lavis.forEachWidget(function(w) w.enabled=true end) end
function lavis.disableAll() lavis.forEachWidget(function(w) w.enabled=false end) end
function lavis.showAll() lavis.forEachWidget(function(w) w.visible=true end) end
function lavis.hideAll() lavis.forEachWidget(function(w) w.visible=false end) end
function lavis.toggleVisibility() lavis.forEachWidget(function(w) w.visible=not w.visible end) end

function lavis.new(...) return lavis.widget(...) end

--[[
	This will initialise some callback functions that Love2d requires.
	To override these functions just add your stuff *below* the lavis calls
	... So that your code never sees them again (unless you make some use of them)
]]

love.mousepressed=love.mousepressed or function(...) lavis.mousepressed(...) end
love.keypressed=love.keypressed or function(...) lavis.keypressed(...) end
love.mousereleased=love.mousereleased or function(...) lavis.mousereleased(...) end
love.keyreleased=love.keyreleased or function(...) lavis.keyreleased(...) end
love.mousemoved=love.mousemoved or function(...) lavis.mousemoved(...) end
love.wheelmoved=love.wheelmoved or function(...) lavis.wheelmoved(...) end
love.update=love.update or function(...) lavis.update(...) end
love.draw=love.draw or function(...) lavis.draw(...) end
love.resize=love.resize or function(...) lavis.resize(...) end
-- love.resize=love.wheelmoved or function(...) lavis.wheelmoved(...) end

return lavis
