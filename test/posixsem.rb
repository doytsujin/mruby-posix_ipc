assert("PSem.new") do
  psem = PSem.open "/hello"
  assert_true  psem.is_a?(PSem)
  assert_equal "/hello", psem.name
  assert_true  psem.named?

  psem.unlink  rescue nil
end

assert("PSem.new w/o name") do
  psem = PSem.new_without_name(value: 3)
  assert_true  psem.is_a?(PSem)
  assert_equal psem.name,  nil
  assert_equal psem.value, 3
  assert_false psem.named?
end

assert("PSem.new default value") do
  psem = PSem.open "/hello_1", value: 3
  assert_equal psem.value, 3

  3.times do
    psem.wait
  end
  assert_equal psem.trywait, -1

  psem.unlink rescue nil
end

assert("PSem#wait") do
  psem = PSem.open "/hello_2"
  p = Process.fork do
    sleep 0.5
    psem.post
  end

  ret = psem.wait
  Process.waitpid p
  assert_equal ret, 0

  psem.unlink rescue nil
end

assert("PSem#trywait") do
  psem = PSem.open "/hello_3"
  assert_equal psem.trywait, -1

  psem.post
  assert_equal psem.trywait, 0

  psem.unlink rescue nil
end
