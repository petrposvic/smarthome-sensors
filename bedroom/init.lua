-- Create web server on port 80. By web interface enable turn LED on (full),
-- on (night mode) or off. Each minute read DHT sensor and one per 5 this reads
-- send average values to the server.

-- Pinouts:
--   D0  = 16;
--   D1  = 5;
--   D2  = 4;
--   D3  = 0;
--   D4  = 2;
--   D5  = 14;
--   D6  = 12;
--   D7  = 13;
--   D8  = 15;
--   D9  = 3;
--   D10 = 1;

name = "bedroom"
httpAddr = "http://influxdb:8086/write?db=smarthome"
httpAuth = "Basic myPrivateAuthToken=="
wifiSSID = "ssid"
wifiPass = "pass"
dhtPin = 4
ledPin = 7

-- Turn LED off
pwm.setup(ledPin, 500, 0)
pwm.start(ledPin)

-- Firstly connect to Wifi
wifi.setmode(wifi.STATION)
wifi.sta.config(wifiSSID, wifiPass)
tmr.alarm(0, 5000, tmr.ALARM_SINGLE, function()
    if (wifi.sta.getip() == nil) then
        print("Wait to IP address!")
    else
        print("New IP address is "..wifi.sta.getip())
        tmr.stop(0)

        -- Small LED blink
        pwm.setduty(ledPin, 127)
        tmr.alarm(0, 200, tmr.ALARM_SINGLE, function()
            pwm.setduty(ledPin, 0)
            tmr.stop(0)
            start(pwm)
        end)
    end
end)

function start(pwm)

    -- Last correctly read values
    last_temp = 0.0
    last_humi = 0.0
    last_time = 0

    -- Server web page, receive requests
    srv=net.createServer(net.TCP)
    srv:listen(80, function(conn)
        conn:on("receive", function(client, request)

        local _, _, method, path, vars = string.find(request, "([A-Z]+) (.+)?(.+) HTTP");
        if (method == nil) then
            _, _, method, path = string.find(request, "([A-Z]+) (.+) HTTP");
        end
        local _GET = {}
        if (vars ~= nil) then
            for k, v in string.gmatch(vars, "(%w+)=(%w+)&*") do
                _GET[k] = v
            end
        end

        if (_GET.pin == "ON") then
            pwm.setduty(ledPin, 1023)
            print("LED on")
        elseif (_GET.pin == "NIGHT") then
            pwm.setduty(ledPin, 127)
            print("LED night")
        elseif (_GET.pin == "OFF") then
            pwm.setduty(ledPin, 0)
            print("LED off")
        end

        local buf = "<style>input {font-size: 4em;}</style>"
        buf = buf.."<h1>Bedroom lights</h1>"
        buf = buf.."<h2>"..last_temp.."*C, "..last_humi.."%</h2>"
        buf = buf.."<h3>updated "..(tmr.time() - last_time).." sec(s) ago</h3>"
        buf = buf.."<form><input type=\"submit\" value=\"RELOAD\" /></form>"
        buf = buf.."<form><input type=\"submit\" name=\"pin\" value=\"OFF\" /></form>"
        buf = buf.."<form><input type=\"submit\" name=\"pin\" value=\"ON\" /></form>"
        buf = buf.."<form><input type=\"submit\" name=\"pin\" value=\"NIGHT\" /></form>"
        client:send(buf)
        client:close()
        collectgarbage();
        end)
    end)

    -- Collected data of DHT sensor, each minute read from the sensor and save
    -- temperature and humidity (odd is temperature, even is humidity)
    data = {}

    -- Each minute increase this variable. If it is 5 or great compute average
    -- temperature and humidity and send it to the server
    count = 0

    if not tmr.alarm(0, 60000, tmr.ALARM_AUTO, function()

        -- Read DHT sensor each minute
        status, temp, humi, temp_dec, humi_dec = dht.read(dhtPin)
        if status == dht.OK then
            print("DHT Temperature:"..temp..";".." Humidity:"..humi)
            table.insert(data, temp)
            table.insert(data, humi)
            last_temp = temp
            last_humi = humi
            last_time = tmr.time()
        elseif status == dht.ERROR_CHECKSUM then
            print("DHT checksum error")
        elseif status == dht.ERROR_TIMEOUT then
            print("DHT timed out")
        end

        -- Send average values each 5 minutes
        count = count + 1
        if (count >= 5 and #data > 1) then
            t_sum = 0.0
            h_sum = 0.0
            for i = 1, #data, 2 do
                t_sum = t_sum + data[i]
                h_sum = h_sum + data[i + 1]
            end
            t_sum = t_sum / (#data / 2)
            h_sum = h_sum / (#data / 2)
            data = {}
            count = 0
            http.post(
                httpAddr,
                "Cache-Control: no-cache\n"..
                "Authorization: "..httpAuth.."\n"..
                "Content-Type: application/json\r\n",
                name.." t="..t_sum..",h="..h_sum..",v=0.0",
                function(code, data)
                    if (code < 0) then
                      print("HTTP request failed")
                    else
                      print(code, data)
                    end
                end)
        end
    end) then
        print("failed to create timer")
    end
end
