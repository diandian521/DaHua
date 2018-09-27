#//////////////////列表/////////////////
m_list = [1,3,5,7,9,9]
print(m_list)

#列表末端添加元素
m_list.append(11)
print(m_list)

#列表插入功能
m_list.insert(1,13)
print(m_list)

#删除功能
m_list.remove(13)
print(m_list)

print(m_list[-2])

print(m_list.index(3))

print(m_list[:3])  #获取前三个元素
print(m_list[2:5]) #获取2——5的元素

print(m_list.index(9)) #获取第一个值为9的元素的索引
print(m_list.count(9)) #统计值为9的元素的个数

m_list.sort() #从小到大排序
print(m_list)

m_list.sort(reverse=True) #从大到小排序
print(m_list)


#//////////////////多维列表/////////////////
multi_list = [[1,2,3,4],[5,6,7,8],[9,10,11,12]]
print(multi_list)
print(multi_list[1][1])

#//////////////////多维列表/////////////////
diri = {'a':1,'b':[1,2,3],'c':{'s1':'apple','s2':'svsun'}}
print(diri)
print(diri['a'])
print(diri['b'])
print(diri['c'])

