#!/usr/bin/env python
# coding: utf-8

# In[4]:


import matplotlib.pyplot as plt # plt 用于显示图片
import matplotlib.image as mpimg # mpimg 用于读取图片
import numpy as np
import cv2


# In[18]:


imglist=['airplane.jpg','boat.jpg','car.jpg','cow.jpg']
for img_name in imglist:
    img = mpimg.imread(img_name) 
    # 此时 img 就已经是一个 np.array 了，可以对它进行任意处理,shape为(?,?,3)
    shrink = cv2.resize(img,(320,240), interpolation=cv2.INTER_AREA)
    plt.imshow(shrink)
    shrink=np.swapaxes(shrink,2,0)#把rgb通道提到前面
    shrink=shrink.astype(np.uint32)
    shrink_rg=np.delete(shrink,2,axis=0)#只有rg通道
    shrink=shrink.ravel()#铺平
    shrink_rg=shrink_rg.ravel()
    #320*240*3个数
    with open('pic_'+img_name+'.txt','w') as f:
        for i in shrink:
            print(hex(i),', ',file=f,end='')
    #320*240*2/4 个数，rg通道，32位为一组16进制数
    with open('pic_'+img_name+'_32bitrg.txt','w') as f:
        num=0
        i=0
        time=0
        while(i<shrink_rg.shape[0]-3):
            for j in range(4):
                num+=shrink_rg[i+j]<<8*(4-j-1)
            time+=1
            print(hex(num),', ',file=f,end='')
            num=0
            i+=4
    #320*240*3/4 个数，32位为一组16进制数
    with open('pic_'+img_name+'_32bit.txt','w') as f:
        num=0
        i=0
        while(i<shrink.shape[0]):
            for j in range(4):
                num+=shrink[i+j]<<8*(4-j-1)
            print(hex(num),', ',file=f,end='')
            num=0
            i+=4

