[![Author](https://img.shields.io/badge/Author-FFOWOTW-blue?style=for-the-badge)](https://github.com/ffowotw/TRScript)
[![Licence](https://img.shields.io/badge/Licence-GNU_v3.0-slime?style=for-the-badge)](https://github.com/ffowotw/STRAsset/blob/main/LICENSE)

# 一個虛幻引擎4的格鬥遊戲插件

TRScript 是一個目前還在 alpha 版的虛幻引擎4用的格鬥遊戲插件。

## 自製的獨立腳本

STRAsset有著 `.trscript` 與 `.collision` 的腳本支援。

`.trscript` 與 `.collision` 皆為 TRScript 的專屬腳本，為製作角色/道具與該碰撞箱的腳本。

## .trscript

`.trscript` 為本插件的角色／道具腳本的檔案類型，用於製作角色與道具。

如需更詳細的資料，請至 https://ffowotw.github.io/trscript/

## .collision

`.collision` 則為本插件的碰撞箱的檔案類型，用於製作角色與道具的碰撞箱。

## Mesh Array 、 Anim Set 與 Anim Array

`Mesh Array` 、 `Anim Set` 與 `Anim Array` 皆可在虛幻引擎4中的內容瀏覽器的選單內的 `STR Asset` 中可以創建。

用途為以下幾點：
- Mesh Array: 為一個Skeletal Mesh的陣列，將角色的拆分多個部分，以方便之後的顯示與隱藏。
- Anim Set: 為每個部分的動畫，動畫的金鑰要與腳本中的名稱相同。
- Anim Array: 為一個Anim Set陣列，將各個部分的Anim Set存入。