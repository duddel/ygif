function init()
    yg.log.info("YGIF: init() called")
end

function tick()
    if yg.input.getDelta("KEY_ESCAPE") > 0.0 then
        yg.control.exit()
    end
end
