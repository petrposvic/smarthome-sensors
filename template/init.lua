function create_temp_ap(msg)
  timeout = 60
  print("creating temporary AP for "..timeout.." seconds")
  print("warning: "..msg)

  tmr.unregister(0)
  tmr.alarm(0, timeout * 1000, tmr.ALARM_SINGLE, function()
    print("disabling AP")
    wifi.setmode(wifi.NULLMODE)
  end)

  cfg = {}
  cfg.ssid = "Temporary"
  cfg.pwd = "password"
  wifi.setmode(wifi.SOFTAP)
  wifi.ap.config(cfg)

  print("creating web server...")
  srv = net.createServer(net.TCP, 30)
  srv:listen(80, function(conn)
    conn:on("receive", function(conn, payload)
      print(payload)

      if string.match(payload, "POST /([a-z]+)") == "wifi" then
        new_cfg = {}
        new_cfg.ssid, new_cfg.pwd = string.match(payload, "ssid=(.*)&pass=(.*)&submit=Save")
        new_cfg.save = true
        -- print(new_cfg.ssid, new_cfg.pwd)

        wifi.setmode(wifi.STATION)
        wifi.sta.config(new_cfg)
        node.restart()
      end
    end)

    conn:on("sent", function(conn)
      conn:close()
    end)

    if file.open("index.html") then
      conn:send(file.read())
      file.close()
    else
      conn:send("can't locate index.html")
    end
  end)
end

-- Connect to Wifi
wifi.setmode(wifi.STATION)
wifi.sta.connect()

-- Get credentials
ssid, pwd = wifi.sta.getconfig()
if ssid == nil then ssid = "?" end
if pwd == nil then pwd = "" end

-- Check wifi status
tmr.alarm(0, 1000, tmr.ALARM_AUTO, function()
  status = wifi.sta.status()
  -- print("Status: "..status)
  if status == wifi.STA_CONNECTING then
    print("connecting to "..ssid)
  elseif status == wifi.STA_WRONGPWD then
    create_temp_ap("cant connect to "..ssid.."; wrong password")
  elseif status == wifi.STA_APNOTFOUND then
    create_temp_ap("cant connect to "..ssid.."; unknown SSID")
  elseif status == wifi.STA_FAIL then
    create_temp_ap("cant connect to "..ssid)
  elseif status == wifi.STA_GOTIP then
    print("connected to "..ssid.."; new IP address is "..wifi.sta.getip())
    tmr.unregister(0)

    if file.exists("main.lua") then
      dofile("main.lua")
    else
      print("file main.lua not found")
    end
  end
end)
