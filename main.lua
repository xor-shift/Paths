require "os"
require "math"

function addMaterialsToScene(scene)
    scene:addMaterial("gray", {
        albedo = point.new({ 0.33, 0.33, 0.33 })
    })
    scene:addMaterial("red", {
        albedo = point.new({ 1, 0, 0 })
    })
    scene:addMaterial("white", {
        albedo = point.new({ 1, 1, 1 })
    })
    scene:addMaterial("mirror", {
        reflectance = 0.75,
        albedo = point.new({ 1, 1, 1 })
    })

    local colorMultiplier = 7;
    local highColor = 5;
    local lowColor = 0.25;
    scene:addMaterial("red light", { emittance = point.new({ highColor, lowColor, lowColor }) * colorMultiplier })
    scene:addMaterial("green light", { emittance = point.new({ lowColor, highColor, lowColor }) * colorMultiplier })
    scene:addMaterial("blue light", { emittance = point.new({ lowColor, lowColor, highColor }) * colorMultiplier })
end

function getLightXYs(radius)
    local arr = { { 0, 0 }, { 0, 0 }, { 0, 0 } }

    arr[1][1] = radius * math.cos(math.pi / 1.5)
    arr[2][1] = radius * math.cos(math.pi / 2.0)
    arr[3][1] = radius * math.cos(math.pi / 3.0)

    arr[1][2] = radius * math.sin(math.pi / 1.5)
    arr[2][2] = radius * math.sin(math.pi / 2.0)
    arr[3][2] = radius * math.sin(math.pi / 3.0)

    return arr
end

Clock = { time = 0 }

function Clock:reset()
    self.time = os.time()
end

function Clock:elapsed()
    return (os.time() - self.time) * 100.0
end

function Clock:new(o)
    o = o or {}
    setmetatable(o, self)

    self.__index = self
    self.time = os.time()

    return o
end

clock = Clock:new(nil)

function main()
    local origin = point.new({ 0, 0, 0 })
    local cameraOffset = point.new({ 0, 6.5, -12 })
    local teapotOffset = point.new({ 0, 0, 0 })
    local mirrorOffset = point.new({ 0, 0, 2.5 })
    local floorOffset = point.new({ 0, 0, 0 })

    local lightXYs = getLightXYs(6)

    local l0Offset = point.new({ lightXYs[1][1], lightXYs[1][2], -1 })
    local l1Offset = point.new({ lightXYs[2][1], lightXYs[2][2], -1 })
    local l2Offset = point.new({ lightXYs[3][1], lightXYs[3][2], -1 })

    local cam = camera.new()
    cam.position = origin + cameraOffset
    cam.fovHint = 100
    cam.resolution = dim2d.new({ 640, 360 })
    cam:setLookAt(origin + teapotOffset + point.new({ 0, 2, 0 }))

    local scene0 = scene.new()
    addMaterialsToScene(scene0)

    clock:reset()
    local teapot = store.newLinearTriFromSTL(
            "objects/teapot.stl",
            scene0:resolveMaterial("white"),
            origin + teapotOffset,
            matrix.newDegRotation(0, -90, 0) * (matrix.newIdentity() * 0.25)
    )
    print("loaded STL in " .. clock:elapsed() .. "ms")

    local treeDepth = 14
    local treeMinShapes = 8
    print("tree arguments: " .. treeDepth .. ", " .. treeMinShapes)

    clock:reset()
    teapot:toFatBVHTri(treeDepth, treeMinShapes)
    print("split up the tree in " .. clock:elapsed() .. "ms")

    clock:reset()
    teapot:toThinBVH()
    print("flattened the tree in " .. clock:elapsed() .. "ms")

    local linearStore = store.newLinear()
    linearStore:insertPlane(scene0:resolveMaterial("mirror"), origin + mirrorOffset, point.new({ 0, 0, -1 }))
    linearStore:insertPlane(scene0:resolveMaterial("gray"), origin + floorOffset, point.new({ 0, 1, 0 }))

    linearStore:insertDisc(scene0:resolveMaterial("red light"), origin + l0Offset, l0Offset - teapotOffset, 1)
    linearStore:insertDisc(scene0:resolveMaterial("green light"), origin + l1Offset, l1Offset - teapotOffset, 1)
    linearStore:insertDisc(scene0:resolveMaterial("blue light"), origin + l2Offset, l2Offset - teapotOffset, 1)

    scene0:getStoreReference():insertChild(linearStore)
    scene0:getStoreReference():insertChild(teapot)

    local integ = integrator.newSamplerWrapper("pt")
    integ:wrapInAverager()
    integ:setCamera(cam)
    integ:setScene(scene0)

    clock:reset()
    local nSamples = 1
    local tStart = os.clock()
    for i = 1, nSamples, 1 do
        integ:tick()
    end
    local msSpent = clock:elapsed()
    print("took " .. msSpent .. "ms for " .. nSamples .. " samples which makes " .. msSpent / nSamples .. "ms spent per sample")

    integ:exportImage("exrf32", "test.exr")
end

main()
