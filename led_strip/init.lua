name = "Name"
wifiSSID = "ssid"
wifiPass = "pass"
ledPin = 4 -- GPIO2

pwm.setup(ledPin, 500, 0)
pwm.start(ledPin)

-- Wait some time after restart
tmr.alarm(0, 2000, tmr.ALARM_SINGLE, function()
    connect()
end)

function connect()
    wifi.setmode(wifi.STATION)
    wifi.sta.config(wifiSSID, wifiPass)
    tmr.alarm(0, 500, tmr.ALARM_AUTO, function()
        if (wifi.sta.getip() == nil) then
            debug("Waiting for IP address")
        else
            debug("Connected")
            tmr.stop(0)
            createServer()
        end
    end)
end

function createServer()
    duty = 0

    srv = net.createServer(net.TCP)
    srv:listen(80, function(conn)
        conn:on("receive", function(client, request)
            local _, _, method, path, vars = string.find(request, "([A-Z]+) (.+)?(.+) HTTP")
            if (method == nil) then
                _, _, method, path = string.find(request, "([A-Z]+) (.+) HTTP")
            end

            local params = {}
            if (vars ~= nil) then
                for k, v in string.gmatch(vars, "(%w+)=([a-zA-Z0-9_\-]+)&*") do
                    params[k] = v
                end
            end

            debug("method = "..method..", path = "..path)
            if (method == "GET") then
                if (params.duty) then
                    local newDuty = tonumber(params.duty)
                    if (params.duty == "ON") then
                        newDuty = 1023
                    elseif (params.duty == "OFF") then
                        newDuty = 0
                    end
                    if (newDuty ~= nil and newDuty >= 0 and newDuty <= 1023) then
                        duty = newDuty
                        pwm.setduty(ledPin, duty)
                    end
                end
            end

            local buf = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<style>input {font-size: 4em;}</style>"
            buf = buf.."<h1>"..name.."</h1>"
            buf = buf.."<h2>"..(duty * 100 / 1023).."% ("..duty..")</h2>"
            buf = buf.."<form><input type=\"submit\" value=\"RELOAD\" /></form>"
            buf = buf.."<form><input type=\"submit\" name=\"duty\" value=\"OFF\" /></form>"
            buf = buf.."<form><input type=\"submit\" name=\"duty\" value=\"ON\" /></form>"
            buf = buf.."<form><input type=\"number\" name=\"duty\" value=\"\" placeholder=\"Duty\" autocomplete=\"off\" /><input type=\"submit\" value=\"Set duty\" /></form>"
            client:send(buf)
        end)
        conn:on("sent", function(conn)
            conn:close()
        end)
    end)
end

function debug(msg)
    -- print(msg)
end
