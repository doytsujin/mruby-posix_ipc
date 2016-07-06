# mruby-posix_ipc   [![Build Status](https://travis-ci.org/udzura/mruby-posix_ipc.svg?branch=master)](https://travis-ci.org/udzura/mruby-posix_ipc)

The POSIX message queue wrapper


## install by mrbgems

- add conf.gem line to `build_config.rb`

```ruby
MRuby::Build.new do |conf|

  # ... (snip) ...

  conf.gem :github => 'udzura/mruby-posix_ipc'
end
```

## examples

### POSIX MQ

```ruby
## One process

mq = PMQ.create('/sample1', mode: "w")
mq.send("The test message!!1")
# => "The test message!!1"

## Another process

mq = PMQ.create('/tmp/sample1', mode: "r")
mq.receive
# => "The test message!!1"
# will block if no message

## Use channel

r, w = PMQ.channel2 "/hello_2"
Process.fork do
  sleep 0.5
  w.send "foobar"
end

res = r.receive
#=> "foobar"
```

#### options for `.create` and `.channel`

* `:mode` specify MQ descriptor read/write mode by letters
* `:queue_size` specify MQ's max size
* `:message_size` specify MQ's max message size, default to `/proc/sys/fs/mqueue/msgsize_max`
* NOTE: `.new` is a raw and low-level API.

## License

under the MIT License:

- see LICENSE file

## See also

- [posix-mqueue MRI gem](https://github.com/Sirupsen/posix-mqueue)
