import matplotlib.pyplot as plt

dists = dict()

def getFloatList(file):
    return [float(i) for i in file.readline().split(",") if i.strip()]

with open("dists", "r") as file:
    dists["uniform"] = getFloatList(file)
    dists["normalBM"] = getFloatList(file)
    dists["normalMP"] = getFloatList(file)
    unitVecDist = getFloatList(file)
    dists["unitVec"] = [unitVecDist[n:n+3] for n in range(0, len(unitVecDist), 3)]

numBins = 128

axSphere = plt.subplot(5, 1, (1, 2), projection="3d")
for x, y, z in dists["unitVec"]:
    axSphere.scatter(x, y, z, marker='.', c=[[0, 0, 0]])

axUni = plt.subplot(5, 1, 3)
axUni.hist(dists["uniform"], label="uniform", bins=numBins)
axNorm = plt.subplot(5, 1, 4)
axNorm.hist(dists["normalBM"], label="normal", bins=numBins)
axNorm = plt.subplot(5, 1, 5)
axNorm.hist(dists["normalMP"], label="normal", bins=numBins)
plt.show()
