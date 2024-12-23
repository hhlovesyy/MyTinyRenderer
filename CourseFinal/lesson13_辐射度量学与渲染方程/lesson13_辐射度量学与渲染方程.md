# lesson13_辐射度量学与渲染方程

## 1 引言

前面的章节我们讲解以及实现了一个较为简单的光线追踪模型，但是很显然，它依旧有很多不真实的地方。因此，我们要学会更精确的光线追踪，而这个前提就是，我们需要先学会能更精确反应现实中的光照信息的辐射度量学，以及渲染方程等。

## 2 辐射度量学

辐射度量学（radiometry）是研究各种电磁辐射（electromagnetic radiation）强弱的学科，研究对于电磁辐射的测量[1]。电磁辐射可被认为是光子组成的粒子流，而电磁辐射的波动形式是电磁波，我们平时看到的可见光就属于电磁波。

![undefined](lesson13_辐射度量学与渲染方程.assets/1280px-EM_spectrum_zh-hans.svg.png)

​						可见光谱只占有宽广的[电磁波谱](https://zh.wikipedia.org/wiki/電磁波譜)的一小部分。图片来自[3]



使用辐射度量学可以从物理上更精准的描述光的信息。

### 2.1 辐射量

要对电磁辐射进行测量，我们需要首先定义一些辐射量（radiometric quantity）。（就比如我们对苹果这个物体的质量进行测量，我们需要定义质量这个物理量一样。）

| 英文名称             | 中文名称        | 符号    | 单位              | 公式                                  | 注解                           |
| -------------------- | --------------- | ------- | ----------------- | ------------------------------------- | ------------------------------ |
| radiant energy       | 辐射能          | $Q$     | $Joule,J$（焦耳） |                                       | 电磁辐射的能量                 |
| radiant flux / power | 辐射通量 / 功率 | $\Phi $ | $watts,W$(瓦特)   | $ \LARGE \frac{dQ}{dt}$               | **单位时间**的辐射能量         |
| irradiance           | 辐照度          | $E$     | $W/m^2$           | $ \LARGE \frac{d \Phi}{d A}$ | 辐射通量相对于**面积**的密度   |
| radiant intensity    | 辐射强度        | $I$     | $W/sr$            | $\LARGE \frac{d \Phi}{d \omega}$                 | 辐射通量相对于**立体角**的密度 |
| **radiance** | 辐射率 | $L$ | $W/m^2sr$ | $\LARGE \frac{d^{2} \Phi }{d A d \omega}$ | 辐射通量相对于**面积和立体角**的密度  / 对**单条光线**中电磁辐射的度量 |

接下来，我们会逐一讲解以上辐射量。



#### 2.1.1 辐射能（radiant energy）

思考：一个灯泡会发光发热，我们该如何表示它的总能量呢？

**辐射能（radiant energy）**就是电磁辐射的能量，符号为$Q$, 单位为$J(Joule), $ 也就是焦耳。电磁辐射可被认为是光子组成的粒子流，因此辐射能也可以认为是光子所携带的能量。

实际上，光源辐射出来的就是辐射能，比如当你打开一个灯泡， 灯泡会发出光，这些光是以电磁辐射的形式传播的能量，这些能量的总量就是辐射能。



#### 2.1.2 辐射通量 （radiant flux） / 功率（power）

思考：一个灯泡单位时间会释放多少能量呢？

**辐射通量 （radiant flux）**又称为**功率（power）**是单位时间的辐射能（radiant energy），符号为$\Phi $，可表示为$\LARGE \frac{dQ}{dt}$， 单位是$watts,W$(瓦特)。

>实际上每个辐射度量学物理量会对应一个光度学物理量，辐射通量 （radiant flux）对应光度学中的光通量（luminous flux），单位是流明（lumen），表示光的亮度。
>
>光度学（photometry）会考虑人眼感知可见光强弱，而人眼对于不同波长的光的敏感度是不同的。
>
>而辐射度量学就不会考虑人眼的感知，而是直接考虑电磁辐射的强弱。

辐射通量 （radiant flux）是辐射度量学中最基本的单位，而非 辐射能（radiant energy）。假设还是一个灯泡照射照射在桌面上，照的时间越长，桌面越热，辐射能（radiant energy）越大，但我们并不想知道总的辐射能，我们其实希望知道，这个灯泡每秒钟会释放多少焦耳的辐射能，因此我们就引入使用辐射通量 （radiant flux）来表示。这个灯泡的辐射通量 （radiant flux） / 功率（power）为100瓦，表示其每秒钟会释放100焦耳的辐射能（radiant energy）。



#### 2.1.3  irradiance（辐照度） 

思考：把手掌放在灯泡旁边，手掌表面接收到的光的能量是多少呢？

> 标题中我们将英文写在前面，因为接下来几个概念大家更习惯用英文来表述，中文可能会引起混淆。

**irradiance（辐照度）** 表示单位面积上的辐射通量，符号为$E$，单位是$W/m^2$（瓦特每平方米）。可以表示为$\LARGE \frac{d \Phi}{d A}$。

具体来说，irradiance（辐照度）表示的是，电磁辐射入射于物体表面时，每单位面积的辐射通量（radiant flux） / 功率（power）。也可以简单理解为用来描述物体表面接收多少光的能量。

在渲染中，这个单位面积一般是物体的表面, 与入射光线垂直的面积。

![image-20241213153150959](lesson13_辐射度量学与渲染方程.assets/image-20241213153150959.png)



如下图，还是之前例子中的灯泡，这个灯泡的辐射通量/ 功率$\Phi $为60W，这个点光源向四周均匀地辐射能量，对于距离光源中心 $r$ 的球壳上，我们取一小块面积$A$，这个面积上的irradiance（辐照度）$E$就是$\LARGE \frac{d \Phi}{d A}$，也就是$\LARGE \frac{\Phi}{4\pi r^2}=\frac{60}{4\pi r^2}$。$r$越大，irradiance（辐照度）越小。

我们可以简单地想象，我们地手掌放在灯泡旁边，这个时候手掌表面单位面积接收到的光的能量就是irradiance（辐照度），我们的手掌距离灯泡越远，感受到的温暖就越小，因为irradiance（辐照度）变小了。

假设手掌距离灯泡1米，那么irradiance（辐照度）的值就是$\LARGE \frac{60}{4\pi r^2}=\frac{60}{4\pi}$ $≈4.77W/m^2$。这个值就是我们手掌表面单位面积接收到的光的能量。



![image-20241219111006211](lesson13_辐射度量学与渲染方程.assets/image-20241219111006211.png)

​														https://yangwc.com/2020/04/06/Spectral/





>在某些地方，使用辐射出射度（radiant emittance，radiant exitance）表示单位面积表面发射的辐射通量。
>
>https://en.wikipedia.org/wiki/Radiant_exitance
>
>公式与irradiance（辐照度）是一致的。



#### 2.1.3  radiant intensity（辐射强度） 

思考：灯泡往一个特定方向发射的能量是多少呢？

**radiant intensity（辐射强度）** 表示单位立体角上的辐射通量，符号为$I$，单位是$W/sr$（瓦特每立体角）。可以表示为$\LARGE \frac{d \Phi}{d \omega}$。

这里提到了立体角的概念，那我们来看看什么是立体角：

##### 立体角

###### 二维角度表示--弧度公式

对于二维的半径为$r$的圆来说，角度可表示为$\theta=\frac{l}{r}$，（弧度公式）

![image-20241219153150774](lesson13_辐射度量学与渲染方程.assets/image-20241219153150774.png)

例如

当$l=2\pi r$, 则$\theta = 2\pi$；

 当$l=\frac{1}{2}\pi r$, 则$\theta = \frac{1}{2}\pi$ .



###### 三维角度表示--立体角

在三维空间中，我们也希望有一个类似于角度的概念，这就是立体角（solid angle），可以理解成从球心指出到球面上的一个很小的面积。、

其公式与二维角度类似，对于半径为$r$的球来说，立体角$\omega$可以表示为 **$\LARGE \omega = \frac{A}{r^2}$**，其中$A$是球面上的面积。

![img](lesson13_辐射度量学与渲染方程.assets/220px-Angle_solide_coordonnees.svg.png)

https://en.wikipedia.org/wiki/Solid_angle

例如：

对于整个球面来说，其面积为$4\pi r^2$，所以整个球面的立体角为$4\pi r^2/r^2=4\pi$ 。

对于半球面来说，其面积为$2\pi r^2$，所以整个球面的立体角为$2\pi r^2/r^2=2\pi$ 。



**立体角公式推导**



实际上在我们后续会遇到的情况中，我们无法直接得到这个立体角对应的面积的大小，而是只知道在球面坐标系下这个立体角的“方向”，即下图中的 $\theta$ 和 $\phi$。那么我们就需要通过 $\theta$ 和 $\phi$来计算面积进而计算立体角。

<img src="lesson13_辐射度量学与渲染方程.assets/image-20241219150009206.png" alt="image-20241219150009206" style="zoom:80%;" />

推导公式的图如下：

下图中，根据上文提到的弧度公式$\theta=\frac{l}{r}$，则$l= r\theta$，我们知道$AB=rd\theta$

<img src="lesson13_辐射度量学与渲染方程.assets/image-20241219150032240.png" alt="image-20241219150032240" style="zoom:80%;" />

如下图，在直角三角形AEO中，$AE=rsin\theta$

<img src="lesson13_辐射度量学与渲染方程.assets/image-20241219151054017.png" alt="image-20241219151054017" style="zoom:80%;" />

如下图，依旧根据上文提到的弧度公式$\theta=\frac{l}{r}$，则$l= r\theta$，我们知道$AC=AE*d\phi =  rsin\theta d\phi$

<img src="lesson13_辐射度量学与渲染方程.assets/image-20241219151600506.png" alt="image-20241219151600506" style="zoom:80%;" />

由此，我们终于得到ABCD的两个边$AB=rd\theta$, $AC=  rsin\theta d\phi$ , 

我们可以算出小“平面”ABCD的面积



那么此时就有：
$$
\begin{aligned}
\mathrm{d} A & =AB*AC=
(r \mathrm{~d} \theta)(r \sin \theta \mathrm{d} \phi)
=r^{2} \sin \theta \mathrm{d} \theta \mathrm{d} \phi \\
\mathrm{d} \omega & =\frac{\mathrm{d} A}{r^{2}}=\sin \theta \mathrm{d} \theta \mathrm{d} \phi
\end{aligned}
$$
![image-20241219152202150](lesson13_辐射度量学与渲染方程.assets/image-20241219152202150.png)

对于整个球来说，有：
$$
\Omega= \int_{s^2}d\omega=\int_{0}^{2\pi}\int_{0}^\pi sin\theta d\theta d\phi = 4\pi
$$


综上，在辐射度量学中,我们直接用ω来表示三维空间中的方向--立体角(ω可以由θ和Φ来进行定义)，$\mathrm{d} \omega =\sin \theta \mathrm{d} \theta \mathrm{d} \phi $  。



回到radiant intensity（辐射强度）上，其定义是单位立体角上的辐射通量，可以表示为$I=\LARGE \frac{d \Phi}{d \omega}$。

由于立体角$\mathrm{d} \omega =\sin \theta \mathrm{d} \theta \mathrm{d} \phi $  ,我们发现， 立体角和半径r是没有关系的，同理，radiant intensity（辐射强度）与距离光源远近无关。见下图

![image-20241211105427418](lesson13_辐射度量学与渲染方程.assets/image-20241211105427418.png)

radiant intensity（辐射强度）并没有发生衰减（因为立体角并没有发生变化，图上红绿色那根线），而是irradiance在发生衰减。





如下图，还是之前例子中的灯泡，这个灯泡向四周均匀地辐射能量，即总的发射辐射通量/ 功率$\Phi$，然后单位立体角发射radiant intensity（辐射强度）$I$ .

因为有:
$$
I=\LARGE \frac{d \Phi}{d \omega}
$$
所以想要求出单位时间的能量Φ,我们就可以对各个立体角方向的$I$进行积分求解, 这就可以得到一个点光源均匀的往四周辐射能量,对应的任何方向的intensity推导公式如下:
$$
\Phi=\int_{S^2}I d\omega=4\pi I (所有立体角的intensity积分起来，能得到功率power)\\  
I=\frac{\Phi}{4\pi}
$$

也就是说，如果是向四周均匀辐射能量的话，任何方向/立体角的Instensity的值都为$\large \frac{\Phi}{4\pi}$。



![image-20241211104053208](lesson13_辐射度量学与渲染方程.assets/image-20241211104053208.png)



我们再举一个例子，如果这次光源不是均匀发光，而是一个聚光灯，发光集中在某方向，假设在某方向上通过的立体角是0.5 sr，功率是60 W，那么：

$I=\LARGE \frac{d \Phi}{d \omega}$$ =  \frac{60}{0.5}=120W/sr$



#### 2.1.4  radiance（辐射率） 

**radiance（辐射率）**是**单位面积**和**单位立体角**上的辐射通量，符号为$L$，单位是$W/m^2sr$（瓦特每平方米每立体角）。可以表示为$\LARGE \frac{d^{2} \Phi }{d A d \omega}$。

radiance（辐射率） 是一个很重要的概念，我们在之前的基础光线追踪章节中，提到的从相机或者眼睛出发，穿过每个像素的光线，就可以用radiance（辐射率）来表示。

![image-20241219192322856](lesson13_辐射度量学与渲染方程.assets/image-20241219192322856.png)



我们可以理解为，radiance是irradiance和radiant intensity的结合，irradiance是单位面积上的辐射通量，radiant intensity是单位立体角上的辐射通量，而radiance是单位面积和单位立体角上的辐射通量。





radiance（L）可以看作是单位面积的radiant intensity（I）。radiant intensity是单位立体角上的辐射通量，我们进一步关注一个微小面积往某个微小立体角上发射的辐射通量，这就是radiance。也可以理解为，这个微小面积dA的radiant intensity是向周围各个方向发射能量，而我们只关心这个微小面积向某个微小立体角\特定方向发射的能量，即radiance。



$\large L=\frac{dI}{dA cos \theta}$

![image-20241219193231903](lesson13_辐射度量学与渲染方程.assets/image-20241219193231903.png)







另一方面，radiance（L）可以看作是单位立体角的irradiance(E)。irradiance是单位面积上接收到的所有方向来的辐射通量，如下图，我们仅仅看这个小面积上接收的一个立体角来的辐射通量，这就是radiance。

$\large L=\frac{dE}{d\omega cos \theta}$

![image-20241219192322856](lesson13_辐射度量学与渲染方程.assets/image-20241219192322856.png)



反之，如果我们将所有方向的立体角的radiance积分起来，就可以得到irradiance:
$$
∵ L_i(\omega)=\frac{dE}{d\omega cos \theta}\\
∴dE(\omega)=L_i(\omega)cos\theta d\omega \\
∴E=\int_{H^2}L_i(\omega)cos\theta d\omega
$$
所以，dA收到的所有能量（也就是irrandiance）就是从各个方向进来的能量求和。其实就是**radiance在irradiance的基础上加了一个方向性**。$H^2$指的是单位半球面





![image-20230416195112331](lesson13_辐射度量学与渲染方程.assets/image-20230416195112331.png)







[1]https://en.wikipedia.org/wiki/Radiometry

[2]real-time rendering 4th

[3]https://zh.wikipedia.org/wiki/%E7%94%B5%E7%A3%81%E6%B3%A2