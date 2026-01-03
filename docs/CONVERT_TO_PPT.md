# 如何將 Markdown 轉換為 PowerPoint

本文件說明如何將 `GAME_MODES_PRESENTATION.md` 轉換為 PowerPoint 簡報格式。

## 方法 1：使用 Pandoc（推薦）

### 安裝 Pandoc
- Windows: 從 https://pandoc.org/installing.html 下載安裝器
- macOS: `brew install pandoc`
- Linux: `sudo apt-get install pandoc`

### 轉換命令
```bash
cd docs
pandoc GAME_MODES_PRESENTATION.md -o Qt_Chess_Game_Modes.pptx
```

### 進階選項
```bash
# 使用自訂範本
pandoc GAME_MODES_PRESENTATION.md -o presentation.pptx --reference-doc=template.pptx

# 指定投影片尺寸
pandoc GAME_MODES_PRESENTATION.md -o presentation.pptx -V aspectratio=169
```

---

## 方法 2：使用 Marp（現代化工具）

### 安裝 Marp CLI
```bash
npm install -g @marp-team/marp-cli
```

### 轉換命令
```bash
cd docs
marp GAME_MODES_PRESENTATION.md -o Qt_Chess_Game_Modes.pptx
```

### 使用 Marp 主題
```bash
marp GAME_MODES_PRESENTATION.md --theme gaia -o presentation.pptx
```

---

## 方法 3：線上轉換工具

### 推薦的線上服務

1. **CloudConvert**
   - 網址：https://cloudconvert.com/md-to-pptx
   - 上傳 `GAME_MODES_PRESENTATION.md`
   - 選擇輸出格式為 PPTX
   - 下載轉換後的文件

2. **Aspose**
   - 網址：https://products.aspose.app/slides/conversion/md-to-pptx
   - 免費線上轉換
   - 支援批量處理

3. **OnlineConverter**
   - 網址：https://www.onlineconverter.com/md-to-pptx
   - 簡單快速
   - 無需註冊

---

## 方法 4：使用 Python 腳本

### 安裝依賴
```bash
pip install python-pptx markdown
```

### 簡易轉換腳本
創建 `convert_md_to_ppt.py`：

```python
from pptx import Presentation
from pptx.util import Inches, Pt
import re

def convert_md_to_ppt(md_file, output_file):
    prs = Presentation()
    
    with open(md_file, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 按投影片分割（使用 --- 分隔符）
    slides_content = content.split('\n---\n')
    
    for slide_text in slides_content:
        if not slide_text.strip():
            continue
            
        slide = prs.slides.add_slide(prs.slide_layouts[1])
        
        # 解析標題和內容
        lines = slide_text.strip().split('\n')
        title = lines[0].replace('#', '').strip()
        
        # 設置標題
        if slide.shapes.title:
            slide.shapes.title.text = title
        
        # 添加內容
        if len(lines) > 1:
            content_text = '\n'.join(lines[1:])
            if len(slide.placeholders) > 1:
                slide.placeholders[1].text = content_text
    
    prs.save(output_file)
    print(f"轉換完成：{output_file}")

# 使用
convert_md_to_ppt('GAME_MODES_PRESENTATION.md', 'Qt_Chess_Game_Modes.pptx')
```

執行：
```bash
python convert_md_to_ppt.py
```

---

## 方法 5：使用 Google Slides

1. 將 `GAME_MODES_PRESENTATION.md` 內容複製
2. 開啟 Google Docs：https://docs.google.com
3. 貼上內容
4. 使用 Google Docs 的「檔案 > 下載 > Microsoft PowerPoint (.pptx)」

或使用 Google Slides Add-on：
- 安裝 "Docs to Slides" 擴充功能
- 直接轉換為 Slides 格式

---

## 轉換後的編輯建議

### 1. 調整投影片佈局
- 檢查每張投影片的排版
- 調整標題和內容的字體大小
- 確保程式碼區塊的可讀性

### 2. 添加視覺元素
- 插入圖表和圖示
- 使用顏色標示重點
- 添加背景圖片

### 3. 優化程式碼顯示
- 使用等寬字體（Consolas, Monaco, Courier New）
- 調整程式碼區塊的背景色
- 可能需要分割過長的程式碼

### 4. 添加動畫效果
- 為重點內容添加進入動畫
- 使用淡入淡出過渡效果
- 控制資訊呈現的節奏

---

## 常見問題

### Q: 轉換後程式碼格式亂掉怎麼辦？
A: 手動調整程式碼區塊，使用等寬字體，可能需要分成多張投影片。

### Q: 中文字體顯示不正確？
A: 確保系統安裝了中文字體（微軟正黑體、思源黑體等），並在 PowerPoint 中手動設置。

### Q: 投影片太多怎麼辦？
A: 可以選擇性刪除部分投影片，或將多張投影片合併。

### Q: 表格顯示不完整？
A: 手動調整表格大小和字體，或將表格轉換為圖片。

---

## 推薦工作流程

1. **使用 Pandoc 快速生成初版**
   ```bash
   pandoc GAME_MODES_PRESENTATION.md -o draft.pptx
   ```

2. **在 PowerPoint 中編輯**
   - 調整佈局和樣式
   - 優化程式碼顯示
   - 添加視覺元素

3. **添加專業元素**
   - 公司/專案 Logo
   - 統一的配色方案
   - 頁碼和頁尾

4. **最終檢查**
   - 預覽每張投影片
   - 檢查拼寫和格式
   - 確保內容完整

---

## 範本下載

可以從以下位置下載 PowerPoint 範本：
- Microsoft Office Templates: https://templates.office.com/
- SlideModel: https://slidemodel.com/
- Free PowerPoint Templates: https://www.free-power-point-templates.com/

---

## 技術支援

如遇到轉換問題，可參考：
- Pandoc 官方文檔：https://pandoc.org/MANUAL.html
- Marp 使用指南：https://marpit.marp.app/
- Stack Overflow 相關問題：https://stackoverflow.com/questions/tagged/pandoc

---

祝您簡報成功！
