import numpy as np
import matplotlib.pyplot as plt

def rand2D():
    """生成[0, 1)之间的随机数"""
    return np.random.rand()

def random_in_unit_disk():
    spx = 2.0 * rand2D() - 1.0  # 随机 x 坐标
    spy = 2.0 * rand2D() - 1.0  # 随机 y 坐标

    r, phi = 0.0, 0.0

    # 确定 r 和 phi 的值来保证生成的点在单位圆盘内
    if spx > -spy:
        if spx > spy:
            r = spx
            phi = spy / spx
        else:
            r = spy
            phi = 2.0 - spx / spy
    else:
        if spx < spy:
            r = -spx
            phi = 4.0 + spy / spx
        else:
            r = -spy
            if spy != 0.0:
                phi = 6.0 - spx / spy
            else:
                phi = 0.0

    phi *= np.pi / 4.0  # 转换为弧度

    return r * np.cos(phi), r * np.sin(phi)  # 返回随机点


def random_double(lower_bound, upper_bound):
    """生成指定范围内的随机浮点数"""
    return lower_bound + (upper_bound - lower_bound) * np.random.rand()


def random_in_unit_disk_refuse():
    """使用拒绝法生成单位圆盘内的随机点"""
    while True:
        # 随机生成一个二维点
        p = np.array([random_double(-1.0, 1.0), random_double(-1.0, 1.0)])

        # 检查是否在单位圆盘内
        if np.linalg.norm(p) < 1.0:
            return p


# 生成 10000 个随机点
num_points = 10000
points = [random_in_unit_disk_refuse() for _ in range(num_points)]
x, y = zip(*points)

# 绘制随机点
plt.figure(figsize=(8, 8))
plt.scatter(x, y, s=1)  # 点的大小设置为1
plt.xlim(-1, 1)
plt.ylim(-1, 1)
plt.title('Random Points in Unit Disk')
plt.xlabel('X-axis')
plt.ylabel('Y-axis')
plt.gca().set_aspect('equal', adjustable='box')  # 使 X 和 Y 轴比例相等
plt.grid(True)
plt.show()
