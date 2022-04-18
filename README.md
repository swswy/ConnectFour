## Background
人工智能导论课程作业，使用了信心上限树搜索和α-β剪枝算法完成
## Usage
运行命令
```shell
g++ -m64 Judge.cpp Strategy.cpp Uct.cpp -fPIC -shared -o Strategy.dll
```
可得到Strategy.dll文件，之后在课程网站上提交此文件即可得到结果