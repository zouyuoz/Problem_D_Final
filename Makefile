# 定義編譯器
CXX = g++

# 編譯器選項
CXXFLAGS = -Wall -g

# 目標程序
TARGET = main

# 源文件
SRCS = main.cpp Band.cpp Channel.cpp Chip.cpp Block.cpp Net.cpp Region.cpp

# 生成的目標文件 (.o)
OBJS = $(SRCS:.cpp=.o)

# 最終生成的可執行文件
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# .cpp 文件對應的 .o 文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(TARGET) $(OBJS)
