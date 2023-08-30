

NAME = http_proxy


# 在这里输入所有cc 文件
CCF = $(wildcard ./src/*.cc) $(wildcard ./lib/*.cc) ./socket++/sockstream.cc

#找到所有的文件夹
dirs:=$(sort $(dir $(CCF)))

#设置.o文件放置的文件夹
build_dir = ./object

out_dir = ./out

#将所有的.cc文件指定到 object 文件夹下
reloc:= $(foreach o,$(CCF),$(build_dir)/$(notdir $o))



# 获得所有的.o文件 和 .d文件
OF = $(reloc:.cc=.o)
DEP = $(reloc:.cc=.d)

# 生成编译的标志引入需要的头文件
inc := $(foreach d,$(dirs),-I$d)
FlAG = -O -Wall -MMD $(inc)

# 告诉搜索路径
VPATH := $(dirs)

all:  $(out_dir)/$(NAME) 


# 告诉编译器没有的文件夹自己创建。
# 告诉makefile 如果要有obj 一定要有对应的文文件夹
$(OF): | $(build_dir) $(out_dir)

# 告诉makefile 没有就自己创建。
$(build_dir): ; mkdir -p $(build_dir)

$(out_dir): ; mkdir -p $(out_dir)

$(out_dir)/$(NAME):$(OF)
	g++ $(FlAG) $^ -o $@ -pthread

-include $(DEP)



$(build_dir)/%.o:%.cc
	g++ -c $(FlAG) $< -o $@ 

.PHONY:clean print pid 

print: 
	# $(OF)
	# $(CCF)
	# $(reloc)

pid:
	ps aux | grep $(NAME)

# cleanup remove outputs and temporary files
clean:
	rm -rf $(out_dir)/$(NAME) out *~ *.bak  $(build_dir)