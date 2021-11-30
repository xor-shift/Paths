require "os"
require "math"
local posix = require "posix"

local function getTimeInMS()
    local sec, nsec = posix.clock_gettime(0)
    return (sec * 1000.0) + (nsec / 1000000.0)
end

local Clock = { time = 0 }

function Clock:new(o)
    o = o or {}
    setmetatable(o, self)

    self.__index = self
    self.time = getTimeInMS()

    return o
end

function Clock:reset()
    self.time = getTimeInMS()
end

function Clock:elapsed()
    return getTimeInMS() - self.time
end

local clock = Clock:new(nil)

local Configuration = {
    integrator = "stat",
    flatteningMethod = 0, -- no flattening, thin, threaded, multiple threaded
    treeDepth = 13,
    treeMinShapes = 8,
    samplesToTake = 16,
    resolution = dim2d.new({ 1280, 720 }),
    outFilename = "",
}

function Configuration:new(o)
    o = o or {}
    setmetatable(o, self)

    self.__index = self
    self.integrator = "stat"
    self.flatteningMethod = 0
    self.treeDepth = 13
    self.treeMinShapes = 8
    self.samplesToTake = 16
    self.resolution = dim2d.new({ 1280, 720 })
    self.outFilename = ""

    return o
end

function Configuration:getOutFilename()
    if self.outFilename ~= "" then
        return self.outFilename
    else
        return self.integrator .. "_" ..
                self.flatteningMethod .. "_" ..
                self.treeDepth .. "," ..
                self.treeMinShapes .. "_" ..
                self.resolution[1] .. "x" .. self.resolution[2] .. ".exr"
    end
end

local Statistics = {
    timeLoad = 0,
    timeConstruct = 0,
    timeFlatten = 0,
    timeRender = 0,
}

function Statistics:new(o)
    o = o or {}
    setmetatable(o, self)

    o.__index = self
    o.timeLoad = 0
    o.timeConstruct = 0
    o.timeFlatten = 0
    o.timeRender = 0

    return o
end

local function doLog(conf, stats)
    print(
            conf.integrator .. "," ..
                    conf.flatteningMethod .. "," ..
                    conf.treeDepth .. "," ..
                    conf.treeMinShapes .. "," ..
                    conf.samplesToTake .. "," ..
                    conf.resolution[1] .. "," .. conf.resolution[2] .. "," ..
                    stats.timeLoad .. "," ..
                    stats.timeConstruct .. "," ..
                    stats.timeFlatten .. "," ..
                    stats.timeRender
    )
end

local model = store.newLinearTriFromSTL(
        "objects/Stanford_Bunny.stl",
--"objects/teapot.stl",
        0,
        point.new({ 0, 0, 0 }),
        matrix.newDegRotation(-180, -90, 0) * (matrix.newIdentity() * 0.03)
)

local function doRender(conf)
    local function getLightXYs(angles, radius, z)
        local arr = {}

        for k, v in pairs(angles) do
            arr[#arr + 1] = point.new({ radius * math.cos(v * (math.pi / 180.0)),
                                        radius * math.sin(v * (math.pi / 180.0)),
                                        z })
        end

        return arr
    end

    local function addMaterialsToScene(scene)
        scene:addMaterial("white", {
            albedo = point.new({ 1, 1, 1 }),
        })
        scene:addMaterial("gray", {
            albedo = point.new({ 0.33, 0.33, 0.33 }),
        })
        scene:addMaterial("red", {
            albedo = point.new({ 1, 0, 0 }),
        })
        scene:addMaterial("mirror", {
            reflectance = 0.9,
            albedo = point.new({ 1, 1, 1 }),
        })
        scene:addMaterial("full mirror", {
            reflectance = 1,
            albedo = point.new({ 1, 1, 1 }),
        })

        local colorMultiplier = 12;
        local highColor = 5;
        local lowColor = 0.25;
        scene:addMaterial("red light", { emittance = point.new({ highColor, lowColor, lowColor }) * colorMultiplier })
        scene:addMaterial("green light", { emittance = point.new({ lowColor, highColor, lowColor }) * colorMultiplier })
        scene:addMaterial("blue light", { emittance = point.new({ lowColor, lowColor, highColor }) * colorMultiplier })
        scene:addMaterial("bright light", { emittance = point.new({ 10, 10, 7 }) })
    end

    local function normaliseImage(img)
        local max = 0
        for i = 1, #img, 1 do
            if img[i]:magnitude() > max then
                max = img[i]:magnitude()
            end
        end
        for i = 1, #img, 1 do
            img[i] = img[i] / max
        end
    end

    local stats = Statistics:new()

    local origin = point.new({ 0, 0, 0 })
    local cameraOffset = point.new({ 0, 6.5, -12 })
    local modelOffset = point.new({ 0, 0, 0 })
    local mirrorOffset = point.new({ 0, 0, 2.5 })
    local floorOffset = point.new({ 0, 0, 0 })

    local lightXYs = getLightXYs({ 120, 90, 60, 30 }, 6, -1)

    local cam = camera.new()
    cam.position = origin + cameraOffset
    cam.fovHint = 100
    cam.resolution = conf.resolution
    cam:setLookAt(origin + modelOffset + point.new({ 0, 2, 0 }))

    local scene0 = scene.new()
    addMaterialsToScene(scene0)

    clock:reset()
    stats.timeLoad = clock:elapsed()

    clock:reset()
    local lModel = model:makeFatBVHTri(conf.treeDepth, conf.treeMinShapes)
    stats.timeConstruct = clock:elapsed()

    clock:reset()
    if conf.flatteningMethod == 1 then
        lModel:toThinBVH()
    elseif conf.flatteningMethod == 2 then
        lModel:toTBVH()
    end
    stats.timeFlatten = clock:elapsed()

    local linearStore = store.newLinear()
    --linearStore:insertPlane(scene0:resolveMaterial("mirror"), origin + mirrorOffset, point.new({ 0, 0, -1 }))
    linearStore:insertPlane(scene0:resolveMaterial("gray"), origin + floorOffset, point.new({ 0, 1, 0 }))

    linearStore:insertDisc(scene0:resolveMaterial("red light"), origin + lightXYs[1], modelOffset - lightXYs[1], 1)
    linearStore:insertDisc(scene0:resolveMaterial("green light"), origin + lightXYs[2], modelOffset - lightXYs[2], 1)
    linearStore:insertDisc(scene0:resolveMaterial("blue light"), origin + lightXYs[3], modelOffset - lightXYs[3], 1)
    --linearStore:insertDisc(scene0:resolveMaterial("bright light"), origin + lightXYs[4], teapotOffset - lightXYs[4], 1)

    scene0:getStoreReference():insertChild(linearStore)
    scene0:getStoreReference():insertChild(lModel)

    local integ = integrator.newSamplerWrapper("stat")
    integ:wrapInAverager()
    integ:setCamera(cam)
    integ:setScene(scene0)

    clock:reset()
    local nSamples = conf.samplesToTake
    for i = 1, nSamples, 1 do
        integ:tick()
        --[[if i % 10 == 0 then
            local perSample = clock:elapsed() / i;
            print(i .. " samples taken, " .. clock:elapsed() .. "ms spent, ETA: " .. (nSamples - i) * perSample .. "ms")
            integ:getImageView():export("test.exr", "exrf32")
        end]]--
    end
    stats.timeRender = clock:elapsed()

    --local img = image.new(integ:getImageView())
    --normaliseImage(img)
    --img:getView():export(conf:getOutFilename(), "exrf32")

    return stats
end

local conf = Configuration:new()
conf.samplesToTake = 64
conf.resolution = dim2d.new({ 8, 8 })

for method = 0, 2, 1 do
    conf.flatteningMethod = method
    for depth = 12,22,1 do
        conf.treeDepth = depth
        local stats = doRender(conf)
        doLog(conf, stats)
        collectgarbage("collect")
    end
end
