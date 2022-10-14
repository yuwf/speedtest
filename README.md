# speedtest
区域块性能监控，支持json、influxdb、prometheus格式的数据快照

# 使用案例
```
函数名和行号作为测试参数
SeedTestObject();
自定义测试参数，第一个参数要求是字面变量或者常量静态区字符串，第二个参数为一个数字
SpeedTestObject2("HandleMsgFun", msgid);

直接使用宏，宏会生成一个静态测试数据对象指针
测试对象会直接使用改指针进行原子操作，效率非常高
```
