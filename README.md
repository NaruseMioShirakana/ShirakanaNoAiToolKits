# 强调

使用GPU（CUDA）版本的工具箱，请安装12.0以下，11.0版本以上的CUDA驱动程序，83.0版本以下的CUDNN动态库，并按照网上的教程安装。

为什么有这样的要求？那就得问CUDA，CUDNN背后的英伟达公司以及OnnxRuntime的官方了，这两个问题都是由CUDA驱动的一些特性和OnnxRuntime的一些问题引起的。

工具箱之前的版本不支持中文路径是什么原因？就是上述问题的一个体现。工具箱本体是支持中文路径的，不过它底层的OnnxRuntime是不支持中文路径的，因为Windows版本的OnnxRuntime使用了Win32Api的A系列函数，A系列函数都是不支持非ANSI编码的路径的。这个问题并不是我能够解决的也不是我应该解决的，只有OnnxRuntime官方修复了这个BUG才可以解决，不过好在最新的OnnxRuntime使用了W系列函数，解决了中文路径的这个问题。

模型加载时候遇到弹窗报错，就是由于上述问题引起，如果引发了这些问题，可以前往https://github.com/microsoft/onnxruntime Onnx官方仓库的Issue查找解决办法。

---

# 用户协议：
- 引用该项目请注明该项目仓库。该项目暂时无法编译（由于使用到的界面库未开源）

- 使用本项目进行二创时请标注本项目仓库地址或作者bilibili空间地址：https://space.bilibili.com/108592413

## 使用该项目代表你同意如下几点：
- 1、你愿意自行承担由于使用该项目而造成的一切后果。
- 2、你承诺不会出售该程序以及其附属模型，若由于出售而造成的一切后果由你自己承担。
- 3、你不会使用之从事违法活动，若从事违法活动，造成的一切后果由你自己承担。
- 4、禁止用于任何商业游戏、低创游戏以及Galgame制作，不反对无偿的精品游戏制作以及Mod制作。
- 5、禁止使用该项目及该项目衍生物以及发布模型等制作各种电子垃圾（比方说AIGalgame，AI游戏制作等）
---

## Q&A：
### Q：该项目以后会收费吗？
    A：该项目为永久免费的项目，如果在其他地方存在本软件的收费版本，请立即举报且不要购买，本软件永久免费。如果想用疯狂星期四塞满白叶，可以前往爱发癫 https://afdian.net/a/NaruseMioShirakana 
### Q：电子垃圾评判标准是什么？
    A：1、原创度。自己的东西在整个项目中的比例（对于AI来说，使用完全由你独立训练模型的创作属于你自己；使用他人模型的创作属于别人）。涵盖的方面包括但不限于程序、美工、音频、策划等等。举个例子，套用Unity等引擎模板换皮属于电子垃圾。

    2、开发者态度。作者开发的态度是不是捞一波流量和钱走人或单纯虚荣。比方说打了无数的tag，像什么“国产”“首个”“最强”“自制”这种引流宣传，结果是非常烂或是平庸的东西，且作者明显没有好好制作该项目的想法，属于电子垃圾。
    
    3、反对一切使用未授权的数据集训练出来的AI模型商用的行为。 
### Q：技术支持？
    A：如果能够确定你做的不是电子垃圾，我会提供一些力所能及的技术支持。 
### 作者的吐槽
    以上均为君子协议，要真要做我也拦不住，但还是希望大家自觉，有这个想法的也希望乘早改悔罢
---

# ShirakanaNoAiToolKits
一个集成了各种有趣和实用AI项目的工具箱，模型下载地址：[HuggingFace](https://huggingface.co/NaruseMioShirakana/ToolKits)

目前支持：
- [PianoTranScription](https://github.com/bytedance/piano_transcription)
- [RealEsrGan](https://github.com/xinntao/Real-ESRGAN)

## 依赖列表
- [FFmpeg](https://ffmpeg.org/)
- [World](https://github.com/JeremyCCHsu/Python-Wrapper-for-World-Vocoder)
- [rapidJson](https://github.com/Tencent/rapidjson) 
- [cxxmidi](https://github.com/5tan/cxxmidi)
