import taichi as ti
import taichi.math as tm
import matplotlib.pyplot as plt

ti.init(arch=ti.gpu)

window = ti.ui.Window("Taichi explicit", (1024, 1024),
                      vsync=True)
canvas = window.get_canvas()
canvas.set_background_color((1, 1, 1))
scene = ti.ui.Scene()
camera = ti.ui.Camera()
intersact_points = []

def sutherland_hodgman_clip(polygon, clipper):
    """
        使用 Sutherland–Hodgman 算法对一个多边形进行裁剪。

        Args:
            polygon (list of tuples): 需要裁剪的多边形顶点列表 [(x1, y1), (x2, y2), ...]
            clipper (list of tuples): 裁剪区域的顶点列表，定义闭合的多边形 [(x1, y1), (x2, y2), ...]

        Returns:
            list of tuples: 裁剪后的多边形顶点列表。
        """
    def inside(point, edge_start, edge_end):
        """判断点是否在裁剪边的内侧"""
        x, y = point
        x1, y1 = edge_start
        x2, y2 = edge_end
        return (x2 - x1) * (y - y1) - (y2 - y1) * (x - x1) <= 0

    def intersect(p1, p2, edge_start, edge_end):
        """计算线段 (p1, p2) 与裁剪边 (edge_start, edge_end) 的交点"""
        x1, y1 = p1
        x2, y2 = p2
        x3, y3 = edge_start
        x4, y4 = edge_end

        a = x2 - x1
        b = x3 - x4
        c = y2 - y1
        d = y3 - y4
        e = x3 - x1
        f = y3 - y1

        # 使用直线的参数方程求交点
        denom = a * d - b * c
        if denom == 0:  # 平行或重合
            return None

        px = (e * d - b * f) / denom
        # py = (a * f - e * c) / denom  # 使用克拉默法则求解
        resx = x1 + px * (x2 - x1)
        resy = y1 + px * (y2 - y1)
        return (resx, resy)

    output_list = polygon
    # 遍历裁剪区域的每一条边
    for i in range(len(clipper)):
        input_list = output_list
        output_list = []

        # 当前裁剪边,clipper是顺时针的
        edge_start = clipper[i]
        edge_end = clipper[(i + 1) % len(clipper)]

        # 遍历多边形的每一条边
        for j in range(len(input_list)):
            current_point = input_list[j]
            prev_point = input_list[(j - 1) % len(input_list)]

            # 判断点的位置
            current_inside = inside(current_point, edge_start, edge_end)
            prev_inside = inside(prev_point, edge_start, edge_end)

            if current_inside:
                if not prev_inside:
                    # 从外部到内部，添加交点
                    intersection = intersect(prev_point, current_point, edge_start, edge_end)
                    if intersection:
                        output_list.append(intersection)
                        intersact_points.append(intersection)
                # 添加当前点
                output_list.append(current_point)
            elif prev_inside:
                # 从内部到外部，添加交点
                intersection = intersect(prev_point, current_point, edge_start, edge_end)
                if intersection:
                    output_list.append(intersection)
                    intersact_points.append(intersection)

    return output_list

def plot_polygon(polygon, color, label):
    x, y = zip(*polygon + [polygon[0]])  # 闭合多边形
    plt.plot(x, y, color=color, label=label)

# 测试代码
if __name__ == "__main__":
    # 多边形的顶点列表（逆时针方向）
    # polygon = [(50, 150), (200, 50), (350, 150), (350, 300), (150, 350), (50, 250)]
    # 上面的改为顺时针方向
    polygon = [(50, 250), (150, 350), (350, 300), (350, 150), (200, 50), (50, 150)]
    # polygon = [(50, 250), (150, 350)]

    # 裁剪区域的顶点列表（顺时针方向）
    clipper = [(100, 100), (100, 400), (300, 400) ,(300, 100)]
    # clipper = [(100, 100), (300, 100), (300, 400), (100, 400)]

    # 调用裁剪算法
    result = sutherland_hodgman_clip(polygon, clipper)

    print("裁剪后的多边形顶点：")
    for point in result:
        print(point)
    for point in intersact_points:
        print(point)
        plt.plot(point[0], point[1], 'ro')
    plt.figure()
    plot_polygon(polygon, "blue", "Original Polygon")
    plot_polygon(clipper, "green", "Clipping Region")
    plot_polygon(result, "red", "Clipped Polygon")

    plt.legend()
    plt.gca().set_aspect('equal')
    plt.show()