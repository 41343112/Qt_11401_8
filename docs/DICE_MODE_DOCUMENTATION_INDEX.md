# 骰子模式文件索引
# Dice Mode Documentation Index

本目錄包含 Qt_Chess 骰子模式的完整程式碼介紹與技術文件。

This directory contains comprehensive code introduction and technical documentation for Qt_Chess Dice Mode.

---

## 📚 文件列表 Document List

### 主要簡報文件 Main Presentation Documents

1. **[骰子模式程式介紹.md](骰子模式程式介紹.md)** (中文版 Chinese Version)
   - 完整的骰子模式程式碼詳細介紹
   - 包含程式碼片段、流程圖、範例演示
   - 適合技術人員深入理解實現細節
   - 可轉換為 PowerPoint 簡報

2. **[Dice_Mode_Code_Presentation_EN.md](Dice_Mode_Code_Presentation_EN.md)** (英文版 English Version)
   - Complete dice mode code introduction
   - Includes code snippets, flowcharts, and examples
   - Suitable for technical staff to understand implementation details
   - Can be converted to PowerPoint presentation

### 技術文件 Technical Documents

3. **[DICE_MODE_CHECK_RULE.md](DICE_MODE_CHECK_RULE.md)**
   - 骰子模式將軍中斷規則詳細說明 (中英對照)
   - Check interruption rule detailed explanation (Bilingual)
   - 技術實現、測試建議、注意事項
   - Technical implementation, testing recommendations, notes

4. **[UPDATED_GAME_MODES.md](UPDATED_GAME_MODES.md)**
   - 遊戲模式總覽 Game modes overview
   - 包含骰子模式、霧戰模式等 Including dice mode, fog of war, etc.

### 功能文件 Feature Documents

5. **[FEATURE_DYNAMIC_DICE_DISPLAY.md](../FEATURE_DYNAMIC_DICE_DISPLAY.md)**
   - 動態骰子顯示更新功能 Dynamic dice display update feature
   - 自動灰階顯示不可用棋子 Auto-greyscale for unavailable pieces

6. **[IMPLEMENTATION_SUMMARY_DICE_CHECK.md](../IMPLEMENTATION_SUMMARY_DICE_CHECK.md)**
   - 骰子模式將軍中斷實現摘要
   - Check interruption implementation summary

### 修復文件 Bug Fix Documents

7. **[BUGFIX_DICE_CHECK_MOVEMENT.md](../BUGFIX_DICE_CHECK_MOVEMENT.md)**
   - 第一次修復：移動驗證 First fix: Move validation

8. **[BUGFIX_DICE_SERVER_SYNC.md](../BUGFIX_DICE_SERVER_SYNC.md)**
   - 第二次修復：伺服器同步 Second fix: Server synchronization

9. **[BUGFIX_DICE_ATOMIC_MESSAGE.md](../BUGFIX_DICE_ATOMIC_MESSAGE.md)**
   - 第三次修復：原子訊息 Third fix: Atomic messaging

---

## 📖 閱讀建議 Reading Recommendations

### 對於想要快速了解骰子模式的讀者 For Quick Overview
推薦閱讀順序 Recommended reading order:
1. README.md (主專案說明 Main project README)
2. 骰子模式程式介紹.md 或 Dice_Mode_Code_Presentation_EN.md
3. DICE_MODE_CHECK_RULE.md

### 對於想要深入技術細節的開發者 For In-Depth Technical Details
推薦閱讀順序 Recommended reading order:
1. 骰子模式程式介紹.md (完整程式碼介紹)
2. IMPLEMENTATION_SUMMARY_DICE_CHECK.md (實現摘要)
3. FEATURE_DYNAMIC_DICE_DISPLAY.md (動態顯示)
4. BUGFIX 系列文件 (了解演進過程)

### 對於想要實現類似功能的開發者 For Developers Implementing Similar Features
推薦閱讀順序 Recommended reading order:
1. 骰子模式程式介紹.md (架構設計)
2. DICE_MODE_CHECK_RULE.md (規則設計)
3. 原始碼檔案：
   - src/qt_chess.h (資料結構)
   - src/qt_chess.cpp (實現細節)
   - src/networkmanager.cpp (網路通訊)

---

## 🎯 文件內容概覽 Document Content Overview

### 骰子模式程式介紹.md 包含 Contains:

#### 第 1 章：概述
- 骰子模式功能說明
- 核心機制與特殊規則

#### 第 2 章：核心資料結構
- 13 個狀態變數詳解
- 資料結構用途說明表

#### 第 3 章：主要功能函數
- 9 個核心函數列表
- 函數職責說明

#### 第 4 章：骰子擲出流程
- 完整流程圖
- rollDiceForTurn() 程式碼詳解

#### 第 5 章：移動驗證機制
- 驗證流程圖
- isPieceTypeInRolledList() 程式碼
- markPieceTypeAsMoved() 程式碼

#### 第 6 章：顯示更新系統
- UI 組件說明
- 顯示狀態對照表
- updateDiceDisplay() 完整程式碼
- canPieceTypeMove() 動態檢查

#### 第 7 章：將軍中斷規則
- 規則詳細說明
- 狀態保存與恢復流程圖
- 檢測與保存程式碼
- 恢復中斷程式碼

#### 第 8 章：網路同步機制
- 網路通訊流程圖
- requestDiceRoll() 函數
- sendMove() 函數（含中斷資訊）
- 伺服器端邏輯 (server.js)

#### 第 9 章：程式碼架構圖
- 完整架構圖
- 模組關係說明

#### 第 10 章：實際範例演示
- 範例 1：正常骰子回合
- 範例 2：將軍中斷完整流程
- 範例 3：動態灰階更新

#### 總結
- 關鍵特點
- 程式碼統計
- 技術亮點

---

## 🔧 如何使用這些文件 How to Use These Documents

### 轉換為 PowerPoint 簡報 Convert to PowerPoint Presentation

您可以使用以下工具將 Markdown 文件轉換為 PPT：
You can use the following tools to convert Markdown to PPT:

1. **Pandoc**（推薦 Recommended）
   ```bash
   pandoc 骰子模式程式介紹.md -o 骰子模式程式介紹.pptx
   ```

2. **Marp**
   - 安裝 Marp CLI Install Marp CLI
   - 在 Markdown 文件開頭加入 Marp 標記 Add Marp markers at the beginning
   - 執行轉換 Execute conversion

3. **在線工具 Online Tools**
   - https://www.markdowntopdf.com/
   - https://dillinger.io/

### 閱讀建議 Reading Tips

- 📱 使用支援 Markdown 的編輯器閱讀 Use Markdown-compatible editors
- 💻 推薦工具：VS Code, Typora, Mark Text
- 🌐 或在 GitHub 上直接預覽 Or preview directly on GitHub

---

## 📊 程式碼統計 Code Statistics

| 項目 Item | 數量 Count |
|-----------|-----------|
| 核心函數 Core Functions | 9 |
| 狀態變數 State Variables | 13 |
| 程式碼行數 Lines of Code | ~600+ |
| 文件總字數 Total Words | ~50,000+ |
| Markdown 文件 MD Files | 9 |

---

## 🎨 簡報特色 Presentation Features

### 骰子模式程式介紹.md 特色：

✅ **完整性** - 涵蓋所有核心功能與實現細節  
✅ **結構化** - 10 章節清晰分類  
✅ **視覺化** - 包含流程圖、架構圖、表格  
✅ **實用性** - 提供真實程式碼範例  
✅ **雙語支援** - 中英文對照  
✅ **可演示** - 包含 3 個完整範例演示  

### Dice_Mode_Code_Presentation_EN.md Features:

✅ **Comprehensive** - Covers all core functions and implementation details  
✅ **Well-Structured** - 10 clear chapters  
✅ **Visual** - Includes flowcharts, architecture diagrams, tables  
✅ **Practical** - Provides real code examples  
✅ **Bilingual** - Chinese-English support  
✅ **Demonstrable** - Includes 3 complete example demonstrations  

---

## 💡 技術亮點 Technical Highlights

### 從文件中可以學到 What You Can Learn:

1. **狀態管理模式** State Management Pattern
   - 如何使用向量管理複雜狀態
   - 保存與恢復機制設計

2. **UI 更新策略** UI Update Strategy
   - 動態樣式切換實現
   - 即時狀態反饋設計

3. **網路同步設計** Network Synchronization Design
   - 客戶端-伺服器通訊協議
   - 原子訊息設計避免競態條件

4. **遊戲規則實現** Game Rule Implementation
   - 複雜規則的狀態機設計
   - 中斷與恢復機制

5. **錯誤處理** Error Handling
   - 邊界條件檢查
   - 防禦性程式設計

---

## 📝 版本資訊 Version Information

- **文件版本 Document Version**: 1.0
- **建立日期 Creation Date**: 2026-01-04
- **程式碼版本 Code Version**: Based on latest commit
- **維護者 Maintainer**: Copilot AI Assistant

---

## 🔗 相關連結 Related Links

- [Qt_Chess 主專案 Main Project](../README.md)
- [線上對戰功能 Online Mode](ONLINE_MODE_FEATURE.md)
- [建置說明 Build Instructions](../BUILDING.md)

---

## 📧 問題回報 Issue Reporting

如果您在閱讀文件時發現任何問題或建議：
If you find any issues or have suggestions while reading:

1. 開啟 GitHub Issue Open a GitHub issue
2. 標註 `documentation` 標籤 Tag with `documentation` label
3. 描述問題或建議 Describe the issue or suggestion

---

**感謝您閱讀骰子模式文件！**  
**Thank you for reading the Dice Mode documentation!**

希望這些文件能幫助您更好地理解 Qt_Chess 骰子模式的實現。  
We hope these documents help you better understand the Qt_Chess Dice Mode implementation.
