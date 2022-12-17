# SerialPro
## 如何在工程中使用
* 本库分为msg_serializer与serialPro两部分
* msg_serializer用于消息序列化与反序列化，是一个纯头文件库，可以通过将`msg_serializer`文件夹拷贝到工程中使用
* serialPro用于串口通信，需要将`thirdparty`、`include`、`msg_serializer`文件夹均拷贝到工程，并编写CMake

## RobotComm API
此类为2023赛季机器人通信协议的C++实现，继承了`serialPro`类，所有配置项均已在构造函数中配置  
只需要实例化本类即可通过`write`函数发送消息，并通过registerCallback方法注册回调函数，
如果解析器扫描到完整消息，会自动调用注册的回调函数

## serialPro API
通过`serialib`库实现串口通信，通过`msg_serializer`库实现消息序列化与反序列化，
仍需要用户配置msg_serializer API中提到的所有配置项，配置项函数均已透传至`serialPro`类  

## msg_serializer API
`msg_serializer`自带了`message_data`宏，所有的消息内容都需要使用该宏进行定义，以关闭自动字节对齐，
  该宏仅对gcc做相应处理，使用其他编译器时需要自行处理  

### Listener
`Listener`是`msg_serializer`中的消息解析器，支持自定义帧头和帧尾  
在进行下述配置后，可以通过`append`方法将数据添加到解析器中，如果解析器扫描到完整消息，会自动调用注册的回调函数  
也可以自行编写新方法将数据添加到`rxBuffer`中，然后调用`scan`方法进行解析  
#### 必要配置
* 必须通过模板传入帧头和帧尾类型，帧头和帧尾类型必须是`message_data`类型（或其他手动关闭字节对齐的类型）
* 必须配置命令字和消息长度获取函数，可以在构造时传入，也可以通过`setGetId`和`setGetLength`函数进行配置
#### 可选配置
* `setMaxSize`设置缓冲区最大长度，默认为1024
* `registerChecker`注册检查器，检查消息是否合法，有单独校验消息头与整包校验两种模式
* `registerCallback`注册回调函数，当解析器扫描到完整消息时会调用命令字对应的回调函数

### Writer
`Writer`是`msg_serializer`中的消息打包器，支持自定义帧头和帧尾  
可以通过`serialize`方法将消息打包成字节流
#### 可选配置
* `registerSetter`注册打包器，主要用于自动处理校验和，有帧头打包器和帧尾打包器两种类型，
  分别对帧头和帧尾中的校验和进行处理

## License信息
本库使用了第三方库[serialib](https://github.com/imabot2/serialib)实现跨平台串口通信，
并对该包做了一定的修改，故继承该包的GPLv3开源协议开源
