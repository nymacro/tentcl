use bindings/editline/editline.so

puts {What is your name?}
set j [readline >]
puts "Hello $j"
