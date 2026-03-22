# 🎨 2D / 3D Graphics – Vector Editor

This project is a vector graphics editor created as part of a computer graphics assignment. The application allows users to create, edit, and manage graphical objects using fundamental rasterization algorithms and geometric transformations.

## ✨ Features

### 🖌️ Shape Drawing
The program supports creating the following shapes:
- Line (defined by two points)
- Quadrilateral (defined by two points)
- N-gon (defined by n points)
- Circle (defined by two points)
- Bézier curve (defined by n control points)

All shapes are stored in a **vector format** (as a set of points).

### 🧱 Layer System & Depth (Z-buffer)
- Each newly drawn shape is rendered **above** the previous ones.
- Every shape has a **depth value** used for Z-buffering.
- Shapes are organized into a **layer list** in the UI (one object per layer).
- Users can reorder shapes by modifying their depth.

### 🎨 Styling
- Global color settings for:
  - Border
  - Fill
- After creation, each shape can have its **own individual border and fill color**.
- Closed shapes (e.g., polygons) can be:
  - Filled
  - Unfilled

### 🧰 Object Manipulation
After selecting a layer, the user can:
- Move (translate) the shape
- Scale the shape
- Rotate the shape

> Transformations are **applied directly to coordinates**, meaning each transformation modifies the shape permanently and future transformations are applied on updated coordinates.

### 🗂️ Object Management
- Every created shape is automatically added to a **list of objects (layers)**.
- The list is visible and interactive in the UI.

### 💾 Save & Load
- The current state of the program can be saved to a file, including:
  - All shapes and their points
  - Colors (border & fill)
  - Depth values
  - Canvas size
- Saved files can be loaded later to continue working.

## ⚙️ Algorithms Used

The project implements the following classic computer graphics algorithms:

- **Line Rasterization:**
  - DDA (Digital Differential Analyzer)
  - Bresenham’s Line Algorithm

- **Circle Rasterization:**
  - Bresenham’s Circle Algorithm

- **Polygon Filling:**
  - Scan-line Algorithm

- **Clipping:**
  - Cyrus-Beck Algorithm
  - Sutherland-Hodgman Algorithm  
  *(Circle clipping is not implemented)*

- **Visibility:**
  - Z-buffer algorithm

## 🧑‍💻 Technologies
- C++ 
- Qt 

