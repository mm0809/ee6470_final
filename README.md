# ee6470_final

## Scalable Merge Sort Platform

I use 2 cores and 4 sorting PE to build my VP.
Each core is responses to assign data to sorting PE and merge 2 small sorted array.

![](https://i.imgur.com/AzR1Ejg.png)

Each sorting PE is designed in module level pipeline and sort 16 elements.

## Synthesis Result of PE
|        | **area** | **latency (1 set data)** | **latency (5 sets data)** |
|:------:|:--------:|:------------------------:|:-------------------------:|
| **v1** |  20339   |           2960           |        6600(1320)         |
| **v2** |  20665   |           2530           |        5250(1050)         |
| **v3** |  25019   |           790            |         1590(318)         |

###### *(v1, v2, v3 are all designed in module level pipeline)*
###### *(number in `()` is average latency)*

From the chart above we can observe that the the latency is shorter when we input more than 1 set of data. Take `v2` as example, when we increase the number of set of input data form 1 to 5, the average latency drop from 2530 to 1050.This is because of module level pipeline design which help us utilize the usage of hardware.


---

|                 area                 |             performance              |
|:------------------------------------:|:------------------------------------:|
| ![](https://i.imgur.com/KoVhH8P.png) | ![](https://i.imgur.com/vXlrFUd.png) |

The char above show the comparison between these design, in `v3` I use some unroll and pipeline pragma so it has a better performance but need more area, but I think it is worth to increase 25% of area for 3.5 times faster performance.


---

|             | **area** | **latency (1 set data)** | **latency (5 sets data)** |
|:-----------:|:--------:|:------------------------:|:-------------------------:|
| **midterm** |   8349   |           700            |         3000(700)         |
|   **v3**    |  25019   |           790            |         1590(318)         |

If we compare with my midterm project version which has no module levle pipeline we can found that module level pipeline design need more area and has longer latency when input only has 1 set. These phenomenons are explainable.

For latency, It is because module level pipeline design need to break down 1 module into several submodules and connect them. Transfer between two modules at least take 1 cycle, consequently pipeline design need more time for transferring than normal design.

For area, in the design of midterm I use 2 arrays which is mapped to register. One for storing input the another one for storing output. In module level pipeline design each stage has it own input and output array, so it need more registers. More register means larger area.

## Software Design
If input has N numbers then divide it into two parts, that is core_0 sort N/2 numbers and core_1 sort N/2. After core_0 and core_1 complete sorting N/2 number, core_0 merge this two N/2 length array then we can get the answer.

## Simulate Result


|                 | 64 numbers | 128 numbers |
| --------------- | ---------- | ----------- |
| simulation time | 197030 ns  | 607100 ns   |

![](https://i.imgur.com/FtEp665.png)
![](https://i.imgur.com/Sp6v2cJ.png)
