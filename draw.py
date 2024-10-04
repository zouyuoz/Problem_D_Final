import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon, Rectangle
from matplotlib.collections import PatchCollection, LineCollection
import numpy as np

# 顏色定義
FEED = '#5050A0' #4040A0
NONFEED = '#A05050' #A04040
REGION = '#608060' #556055
ZONEEDGE = 'dimgray'
BPR = '#C06030'
TBENN = '#6030C0'
MT = '#80FFFF'
HMFT = '#FF80FF'
CHANNEL = '#A0D0A0'
NET = 'darkgreen'

def read_polygons(filename):
    df = pd.read_csv(filename)
    polygons = []
    colors = []
    for group in df['group'].unique():
        polygon = df[df['group'] == group]
        poly = Polygon(polygon[['x', 'y']].values, closed=True)
        polygons.append(poly)
        is_feed = polygon['is_feed'].iloc[0]
        colors.append(FEED if is_feed==1 else (NONFEED if is_feed==0 else REGION))
    return polygons, colors

def read_must_throughs(filename):
    df = pd.read_csv(filename)
    lines = df[['x1', 'y1', 'x2', 'y2']].values.reshape(-1, 2, 2)
    colors = [TBENN if h==1 else (MT if h==2 else (HMFT if h==3 else BPR)) for h in df['mt']]
    return lines, colors

def read_net(filename):
    df = pd.read_csv(filename)
    lines = df[['x1', 'y1', 'x2', 'y2']].values.reshape(-1, 2, 2)
    return lines

def read_rectangles(filename):
    df = pd.read_csv(filename)
    rectangles = []
    colors = []
    for _, row in df.iterrows():
        x1, y1, x2, y2 = row['x1'], row['y1'], row['x2'], row['y2']
        bpr = row['bpr']
        colors.append('dodgerblue' if bpr==1 else ('violet' if bpr==2 else (CHANNEL if bpr==0 else BPR)))
        rectangles.append(Rectangle((x1, y1), x2 - x1, y2 - y1))  # Create Rectangle object
    return rectangles, colors

def read_points(filename):
    df = pd.read_csv(filename)
    points = df[['x', 'y']].values
    return points

def plot_data(is_zzb, is_zzm, is_zzn, is_zzp):
	fig, ax = plt.subplots(figsize=(14, 12.5))
	# ax.set_facecolor('k')

	# 繪製多邊形
	if is_zzb:
		polygons, poly_colors = read_polygons('zzb.csv')
		p = PatchCollection(polygons, edgecolor=ZONEEDGE, alpha=0.2)
		p.set_facecolors(poly_colors)
		ax.add_collection(p)

    # 繪製must_throughs
	if is_zzm:
		must_through_lines, mt_colors = read_must_throughs('zzm.csv')
		lc = LineCollection(must_through_lines, colors=mt_colors, linewidths=1)
		ax.add_collection(lc)

	# 繪製net
	if is_zzn:
		lines = read_net('zzn.csv')
		lc = LineCollection(lines, colors=NET, lw=0.5)
		ax.add_collection(lc)
  
	# 繪製長方形
	if is_zzp:
		rectangles, rec_colors = read_rectangles('zzp.csv')
		p = PatchCollection(rectangles, alpha=0.2, lw = 0.5) # 
		p.set_facecolor(rec_colors)
		# p.set_edgecolor(ZONEEDGE)
		points = read_points('zzi.csv')
		ax.scatter(points[:, 0], points[:, 1], color=ZONEEDGE, marker=".", s=10.5)
		ax.add_collection(p)

	ax.autoscale()
	plt.title('Chip top Visualization')
	plt.xlabel('X coordinate')
	plt.ylabel('Y coordinate')

	# 添加圖例
	feed    = plt.Rectangle((0, 0), 1, 1, fc=FEED,    alpha=0.2, ec=ZONEEDGE)
	nonfeed = plt.Rectangle((0, 0), 1, 1, fc=NONFEED, alpha=0.2, ec=ZONEEDGE)
	region  = plt.Rectangle((0, 0), 1, 1, fc=REGION,  alpha=0.2, ec=ZONEEDGE)
	channel = plt.Rectangle((0, 0), 1, 1, fc=CHANNEL, alpha=0.2, ec=ZONEEDGE)
	bpr_cha = plt.Rectangle((0, 0), 1, 1, fc=BPR,     alpha=0.2, ec=ZONEEDGE)
	tbenn   = plt.Line2D([0], [0], color=TBENN, linewidth=2)
	bpr     = plt.Line2D([0], [0], color=BPR,   linewidth=2)
	net_    = plt.Line2D([0], [0], color=NET,   linewidth=2)
	mt      = plt.Line2D([0], [0], color=MT,    linewidth=2)
	hmft    = plt.Line2D([0], [0], color=HMFT,  linewidth=2)
    
	plt.legend([feed, nonfeed, region, tbenn, bpr, net_, mt, hmft, channel, bpr_cha], 
			   ['Feed', 'NonFeed', 'Region', 'TBENN', 'BPR', 'NET', 'MT', 'HMFT MT', 'Channel', 'BPR-Cha'], 
			   loc='upper left', bbox_to_anchor=(1, 1))

	plt.tight_layout()
	plt.show()

# 使用示例，四個boolean分別代表
# 輸出blocks, 輸出ports, 輸出net(s), 輸出cells
plot_data(0,1,1,1)