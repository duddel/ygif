function init()
    yg.log.info("YGIF: init() called")

    -- load assets
    geoCube = yg.gl.loadGeometry("a//cube.obj")
    geoGrid = yg.gl.loadGeometry("a//grid.obj")
    shdrDiff = yg.gl.loadVertFragShader("a//default.vert", "a//diffusecolor.frag")
    shdrSimple = yg.gl.loadVertFragShader("a//default.vert", "a//simplecolor.frag")

    -- make camera and position it in scene
    c = yg.math.Camera()
    eye = {}
    eye[1] = 7.35889
    eye[2] = 4.95831
    eye[3] = 6.92579
    center = {}
    center[1] = 0.0
    center[2] = 0.0
    center[3] = 0.0
    up = {}
    up[1] = 0.0
    up[2] = 1.0
    up[3] = 0.0
    c:trafo():lookAt(eye, center, up)

    -- make lightsource
    light = yg.gl.Lightsource()
    lightPos = {}
    lightPos[1] = 4.07625
    lightPos[2] = 5.90386
    lightPos[3] = -1.00545
    light:setPosition(lightPos)
    lightDiffuse = {}
    lightDiffuse[1] = 1.0
    lightDiffuse[2] = 1.0
    lightDiffuse[3] = 1.0
    light:setDiffuse(lightDiffuse)

    -- dummy model Trafo (identity)
    t = yg.math.Trafo()
    t:setIdentity()
end

function tick()
    yg.gl.draw(geoCube, light, shdrDiff, c, t)
    yg.gl.draw(geoGrid, nil, shdrSimple, c, t)

    if yg.input.getDelta("KEY_ESCAPE") > 0.0 then
        yg.control.exit()
    end
end
