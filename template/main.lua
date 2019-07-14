srv = net.createServer(net.TCP, 30)
srv:listen(80, function(conn)
  conn:on("receive", function(conn, payload)
    print(payload)
  end)

  conn:on("sent", function(conn)
    conn:close()
  end)

  conn:send("TODO")
end)