# 工程名
TARGET		  	?= http_server
# 编译器
CC 				:= g++
# 头文件路径
HEADDIR 		:= header
# 源文件路径		   			   
CPPDIR			:= sources

# 得到obj文件的名字
CPPFILES		:= $(wildcard $(CPPDIR)/*.cpp) # 选出文件夹中所有的cpp文件
CPPFILENAME		:= $(notdir  $(CPPFILES)) # 去掉路径，保留文件名
OBJS			:= $(patsubst %, obj/%, $(CPPFILENAME:.cpp=.o)) # 将cpp文件扩展名，替换为.o

# 连接 
$(TARGET): $(OBJS)
	$(CC) -g -o $(TARGET) $^ -lpthread 

# 编译
$(OBJS): obj/%.o: $(CPPDIR)/%.cpp
	$(CC) -Wall -c -I$(HEADDIR) -std=c++11 -o $@ $<


.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJS)