# Multiple plot function
#
# ggplot objects can be passed in ..., or to plotlist (as a list of ggplot objects)
# - cols:   Number of columns in layout
# - layout: A matrix specifying the layout. If present, 'cols' is ignored.
#
# If the layout is something like matrix(c(1,2,3,3), nrow=2, byrow=TRUE),
# then plot 1 will go in the upper left, 2 will go in the upper right, and
# 3 will go all the way across the bottom.
#
multiplot <- function(..., plotlist=NULL, file, cols=1, layout=NULL) {
  library(grid)
  
  # Make a list from the ... arguments and plotlist
  plots <- c(list(...), plotlist)
  
  numPlots = length(plots)
  
  # If layout is NULL, then use 'cols' to determine layout
  if (is.null(layout)) {
    # Make the panel
    # ncol: Number of columns of plots
    # nrow: Number of rows needed, calculated from # of cols
    layout <- matrix(seq(1, cols * ceiling(numPlots/cols)),
                     ncol = cols, nrow = ceiling(numPlots/cols))
  }
  
  if (numPlots==1) {
    print(plots[[1]])
    
  } else {
    # Set up the page
    grid.newpage()
    pushViewport(viewport(layout = grid.layout(nrow(layout), ncol(layout))))
    
    # Make each plot, in the correct location
    for (i in 1:numPlots) {
      # Get the i,j matrix positions of the regions that contain this subplot
      matchidx <- as.data.frame(which(layout == i, arr.ind = TRUE))
      
      print(plots[[i]], vp = viewport(layout.pos.row = matchidx$row,
                                      layout.pos.col = matchidx$col))
    }
  }
}

plot_roc1 <- function(last_line_number = 40, roc_file = "~/ClionProjects/okcoin_bot/cmake-build-debug/roc") {
  library(reshape2)
  library(ggplot2)
  roc <- read.table("~/ClionProjects/okcoin_bot/cmake-build-debug/roc", sep="\t",  header = FALSE)
  colnames(roc) <- c("time", "kline", "roc", "maroc")
  roc_without_kline <- roc[, -which(names(roc) %in% c("kline"))]
  start_line <- 1
  total_line <- nrow(roc)
  if (total_line > last_line_number) {
    start_line <- total_line - last_line_number + 1
  }
  cutted_roc_without_kline <- roc_without_kline[c(start_line:total_line),]
  cutted_md <- melt(cutted_roc_without_kline, id=c("time"))
  roc_only_kline <- roc[, which(names(roc) %in% c("time", "kline"))]
  cutted_kline <- roc_only_kline[c(start_line:nrow(roc)),]
  variable1 <- rep(1, last_line_number)
  p1 <- ggplot(cutted_md, aes(x=time, y=value, colour=variable, group=variable)) + geom_point() + geom_line()
  p2 <- ggplot(cutted_kline, aes(x=time, y=kline, group=1, colour=variable1)) + geom_point() + geom_line()
  multiplot(p1, p2)
}

plot_roc2 <- function(start_line, end_line, roc_file = "~/ClionProjects/okcoin_bot/cmake-build-debug/roc") {
  
  library(reshape2)
  library(ggplot2)
  roc <- read.table("~/ClionProjects/okcoin_bot/cmake-build-debug/roc", sep="\t",  header = FALSE)
  if (start_line < 1 || end_line > nrow(roc) || start_line > end_line) {
    cat("illegal line number")
    return()
  }
  
  colnames(roc) <- c("time", "kline", "roc", "maroc")
  roc_without_kline <- roc[, -which(names(roc) %in% c("kline"))]
  
  cutted_roc_without_kline <- roc_without_kline[c(start_line:end_line),]
  cutted_md <- melt(cutted_roc_without_kline, id=c("time"))
  roc_only_kline <- roc[, which(names(roc) %in% c("time", "kline"))]
  cutted_kline <- roc_only_kline[c(start_line:end_line),]
  variable1 <- rep(1, end_line - start_line + 1)
  p1 <- ggplot(cutted_md, aes(x=time, y=value, colour=variable, group=variable)) + geom_point() + geom_line()
  p2 <- ggplot(cutted_kline, aes(x=time, y=kline, group=1, colour=variable1)) + geom_point() + geom_line()
  multiplot(p1, p2)
}