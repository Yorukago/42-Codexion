# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/05/29 13:28:38 by jzorreta          #+#    #+#              #
#    Updated: 2026/06/25 23:41:26 by jzorreta         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME        = codexion

PINK        = \033[1;35m
RESET       = \033[0m

CC          = cc
CFLAGS      = -Wall -Wextra -Werror -pthread
INC         = -I includes/

SRC_DIR     = sources/
OBJ_DIR     = obj/

SRC_FILES   = codexion.c parser.c init.c logs.c heap2.c \
              coder.c monitor.c dongle.c scheduler.c heap.c \
			  cleanup.c

SRCS        = $(addprefix $(SRC_DIR), $(SRC_FILES))
OBJS        = $(addprefix $(OBJ_DIR), $(SRC_FILES:.c=.o))

	
define ASCII_ART

                   ,gggg,                                                                                                     
                 ,88"""Y8b,                      8I                                                                           
                d8"     `Y8                      8I                                                                           
               d8'   8b  d8                      8I                          gg                                               
              ,8I    "Y88P'                      8I                          ""                 g                              
 aaaaaaaa     I8'             ,ggggg,      ,gggg,8I   ,ggg,      ,gg,   ,gg  gg     ,ggggg,     ggggg'     aaaaaaaa     
 """"""""     d8             dP"  "Y8ggg  dP"  "Y8I  i8" "8i    d8""8b,dP"   88    dP"  "Y8ggg ,8" "8P'    """"""""     
              Y8,           i8'    ,8I   i8'    ,8I  I8, ,8I   dP   ,88"     88   i8'    ,8I   I8   8I                     
              `Yba,,_____, ,d8,   ,d8'  ,d8,   ,d8b, `YbadP' ,dP  ,dP"Y8,  _,88,_,d8,   ,d8'  ,dP   8I                     
                `"Y8888888 P"Y8888P"    P"Y8888P"`Y8888P"Y8888"  dP"   "Y888P""Y8P"Y8888P"    8P'   8I                    
                                                                                                                              
endef
export ASCII_ART

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) $(INC) -o $(NAME)
	@printf "$(PINK)$$ASCII_ART$(RESET)\n"
	@printf "$(PINK)             					    Codexion is ready! $(RESET)\n"

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR)
	@printf "$(PINK)Objects vanished!$(RESET)\n"

fclean: clean
	@rm -f $(NAME)
	@printf "$(PINK)Executables destroyed!$(RESET)\n"

re: fclean all

tests: all
	@chmod +x tests.sh
	@./tests.sh

.PHONY: all clean fclean re tests