function init()
    yg.log.info("ygif init()...")
    yg.control.enableVSync(true)

    -- initial time
    time0 = yg.time.getTime()

    -- load assets
    geoCube = yg.gl.loadGeometry("a//cube.obj")
    geoGrid = yg.gl.loadGeometry("a//grid.obj")
    shdrDiff = yg.gl.loadVertFragShader("a//default.vert", "a//diffusecolor.frag")
    shdrSimple = yg.gl.loadVertFragShader("a//default.vert", "a//simplecolor.frag")

    -- initialize audio
    yg.audio.init(2, 44100, 5)
    yg.audio.storeFile("a//laserSmall_000.ogg")

    -- make camera and position it in scene
    c = yg.math.Camera()
    eye = {};
    eye[1] = 7.35889; eye[2] = 4.95831; eye[3] = 6.92579
    center = {}
    center[1] = 0.0; center[2] = 0.0; center[3] = 0.0
    up = {}
    up[1] = 0.0; up[2] = 1.0; up[3] = 0.0
    c:trafo():lookAt(eye, center, up)

    -- make lightsource
    light = yg.gl.Lightsource()
    lightPos = {}
    lightPos[1] = 4.07625; lightPos[2] = 5.90386; lightPos[3] = -1.00545
    light:setPosition(lightPos)

    -- dummy model Trafo (identity)
    t = yg.math.Trafo()
    t:setIdentity()
end

function tick()
    -- update light 
    lightDiffuse = yg.flavor.getVec3("tint")
    light:setDiffuse(lightDiffuse)

    -- update camera from input
    c:setPerspective(40, yg.input.get("WINDOW_ASPECT_RATIO"), 1, 100)
    c:trafo():rotateGlobal(yg.time.getDelta() * 0.75 * yg.input.get("KEY_LEFT"), "Y")
    c:trafo():rotateGlobal(yg.time.getDelta() * -0.75 * yg.input.get("KEY_RIGHT"), "Y")
    c:trafo():rotateLocal(yg.time.getDelta() * 0.75 * yg.input.get("KEY_UP"), "X")
    c:trafo():rotateLocal(yg.time.getDelta() * -0.75 * yg.input.get("KEY_DOWN"), "X")

    -- update cube Trafo
    t:rotateGlobal(yg.time.getDelta() * math.pi * 2 * yg.flavor.getNumber("rotation"), "Y")
    cubeTrans = yg.flavor.getVec3("position")
    cubeTrans[2] = cubeTrans[2] + math.sin((yg.time.getTime() - time0) * math.pi * 2 * yg.flavor.getNumber("bounce")) * 0.5
    t:setTranslation(cubeTrans)

    -- play audio
    if yg.input.getDelta("KEY_SPACE") > 0.0 then
        yg.audio.play("a//laserSmall_000.ogg")
    end

    -- draw
    yg.gl.draw(geoCube, light, shdrDiff, c, t)
    yg.gl.draw(geoGrid, nil, shdrSimple, c, nil)
end

function shutdown()
    yg.log.info("ygif shutdown()...")

    -- todo add mechanism to delete assets

    yg.audio.shutdown()
end
