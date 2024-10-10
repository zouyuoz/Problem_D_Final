# Problem_D - ICCAD Contest 2024

## 作者
國立陽明交通大學 半導體工程學系
- 邱照元
- 鄭秉睿

*請尊重智慧財產權*

## 使用方法

### 1. 執行程式

a. 首先，打開 `main.cpp` 文件，在 `main` 函數的第一行修改要執行的 test case。

b. 在命令行中執行以下指令：

```
cd "[path to this folder(...\Problem_D_Final)]" ; if ($?) { g++ main.cpp Algorithm.cpp Cell.cpp Chip.cpp Block.cpp Net.cpp Region.cpp -o main} ; if ($?) { .\main [tracks/um] }
```

如

```
cd "User\Downloads\Problem_D_Final" ; if ($?) { g++ main.cpp Algorithm.cpp Cell.cpp Chip.cpp Block.cpp Net.cpp Region.cpp -o main} ; if ($?) { .\main 50 }
```

### 2. 輸出

針對每個case，會輸出相對應的檔案 `caseXX_net.rpt`，為正式輸出格式，可使用此檔案作為後續的 evaluation。

### 3. 視覺化 chip top

如果在 `main` 函數中執行了 chip top 的 visualization（將 `visualizeChip` 設為 `true`），相關數據會被寫入多個 CSV 文件中。

#### 視覺化步驟：
1. 確保已安裝所需的 Python 函式庫：
   - pandas
   - matplotlib

2. 打開 `draw.py` 文件。

3. 在文件最後的 `plot_data` 函數中，選擇要輸出的數據類型並修改相應的 boolean 值。

4. 執行 `draw.py`，稍等片刻後即可看到生成的圖表。

## 注意事項
- 執行 `draw.py`前，請確保已正確安裝所有必要的 Python 庫。
- 如遇到任何問題，請檢查文件路徑和執行權限。
