-- Debug change and save WiFi credentials
cfg = {}
cfg.ssid = "ssid"
cfg.pass = "pass"
cfg.save = true
wifi.sta.config(cfg)
