# Bus RoutePicker: 南京邮电大学公交路线自动选择系统

A bus route automatic selection system developed for Nanjing University of Posts and Telecommunications' Programming Week.


## 📋 Project Overview
This system provides comprehensive bus route query and management functions for university campuses, supporting both user and administrator roles with features like route search, favorite management, and route data maintenance.


## ✨ Key Features

### 🔍 User Functions
- Search for direct and transfer routes between stations
- View all bus routes with detailed information
- Check route maps (requires pre-prepared PNG files)
- Manage favorite routes (create, edit, delete, and reuse favorites)


### 🔧 Administrator Functions
- Add, modify, and delete bus routes
- Backup and restore route data
- Register new administrators
- Initialize user accounts
- View system-wide route overview


## 🚀 Technical Details
- **Language**: C
- **Libraries**: EasyX (for map display), Windows API (for file operations)
- **Data Storage**: Text files (`users.txt` for user data, `routes.txt` for route data)
- **Features**: 
  - Real-time arrival time calculation
  - Route validity check
  - Password input masking
  - File-based data persistence


## 📁 Project Structure
- Core data structures for users and bus routes
- Route search algorithm (direct and transfer routes)
- Time calculation module
- User management system
- Favorite route management
- Graphical map display


## 📝 Usage Instructions
1. Compile with a C compiler supporting EasyX library
2. Initialize with default admin account (`admin` / `123456`)
3. Prepare route map images as `[route_name].png`
4. Create `collection` folder for favorite routes storage


## 👥 Developers
- Nanjing University of Posts and Telecommunications students
- Developed for Programming Week project

---

*Note: This system is designed for educational purposes and campus bus route management.*
