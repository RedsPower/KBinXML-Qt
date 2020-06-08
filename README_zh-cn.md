# KBinXML-Qt

0. 简介

   此项目是[mon](https://github.com/mon/)的[KBinXML](https://github.com/mon/kbinxml)的Qt移植版本。

1. 用法

   用法与原始版本类似。

   示例（假设输入为in，类型为QByteArray）：

   KBin -> XML：

   ```c++
   KBinXML kbin(in);
   if(kbin.loaded())
   	qDebug() << kbin.toXML();
   ```
   XML -> KBin：
   
   ```c++
   KBinXML kxml(in);
   qDebug() << kxml.toBin();
   ```

2. 备注

   对于KBin  -> XML，可使用 `loaded()`方法确认加载是否成功，用`xmlEncoding()`方法取得KBin的编码；
   
   对于XML -> KBin，可在调用`toBin()`方法的时候传入编码名字以设置输出的KBin编码（默认SHIFT-JIS编码）（XML的编码是从XML头部读取的，否则默认UTF-8）
   
   如果传入不在可用编码列表里的编码名字，结果是未定义的
   
   （注：`xmlEncoding()`虽然在XML -> KBin时也可用，但它好像没啥意义（（
   
   `toXML()`函数返回的是以UTF-8方式存储的QByteArray，并加上了XML头部
   
   
   
   
   
   