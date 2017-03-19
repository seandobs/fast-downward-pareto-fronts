library(ggplot2)
library(ggpmisc)
library(grid)
library(gridExtra)

outputs="./outputs/tight"

results <- read.csv(paste(outputs,"results",sep="/"),sep="",header=T)
results <- subset(results, found == 1)
results$evaluations <- as.numeric(as.character(results$evaluations))
results$searchtime <- as.numeric(as.character(results$searchtime))

#results <- subset(results, domain != "woodworking-strips")

results$pernode <- results$searchtime / results$evaluations

ipdb <- subset(results, heuristic == "ipdb-ework-sum")
ipfpdb <- subset(results, heuristic == "ipfpdb-ework-sum")

inter <- merge(x=ipdb, y=ipfpdb, by=c("domain","problem"))

png(paste(outputs,"pernode.png",sep="/"), width = 1920, height=1080, res=300)

f <- y ~ x
p1 <- ggplot(inter, aes(x = pernode.x, y=pernode.y)) +
    scale_shape_manual(values=1:nlevels(inter$domain)) +
    scale_linetype_manual(name="Fit Line", values=c("Data Best Fit"="dashed", "1:1"="solid")) +
    geom_abline(aes(linetype="1:1", intercept = 0, slope = 1), size=0.5) +
    geom_smooth(aes(linetype="Data Best Fit"), method = "lm", se = FALSE, size=0.5, show.legend=FALSE, formula = f) +
    stat_poly_eq(formula = f, 
                aes(label = paste(..eq.label.., ..rr.label.., sep = "~~~")), 
                parse = TRUE) +
    geom_point(aes(shape=domain, colour=domain)) +
    labs(x="iPDB Time/Node Evaluation (s)",
         y="iPFPDB Time/Node Evaluation (s)",
         shape="Domain",colour="Domain")

p1

dev.off()


png(paste(outputs,"evaluations.png",sep="/"), width = 1920, height=1080, res=300)

p2 <- ggplot(inter, aes(x = evaluations.x, y=evaluations.y)) +
    scale_shape_manual(values=1:nlevels(inter$domain)) +
    scale_linetype_manual(name="Fit Line", values=c("Data Best Fit"="dashed", "1:1"="solid")) +
    geom_abline(aes(linetype="1:1", intercept = 0, slope = 1), size=0.5) +
    geom_smooth(aes(linetype="Data Best Fit"), method = "lm", se = FALSE, size=0.5, show.legend=FALSE, formula = f) +
    stat_poly_eq(formula = f, 
                aes(label = paste(..eq.label.., ..rr.label.., sep = "~~~")), 
                parse = TRUE) +
    geom_point(aes(shape=domain, colour=domain)) +
    labs(x="iPDB No. Evaluations",
         y="iPFPDB No. Evaluations",
         shape="Domain",colour="Domain")

dev.off()

inter$diff <- inter$evaluations.x - inter$evaluations.y

png(paste(outputs,"diffevaluations.png",sep="/"), width = 1920, height=1080, res=300)

ggplot(inter, aes(x = problem, y=diff, color = domain, shape = domain)) +
    scale_shape_manual(values=1:nlevels(inter$domain)) +
    geom_point() +
    labs(x="Problem (If solved by both)", y="(iPDB - iPFPDB) Evaluations", color="Domain",shape="Domain") +
    theme(axis.text.x = element_text(angle=45, hjust=1))

dev.off()


png(paste(outputs,"pernodeevals.png",sep="/"), width = 2500, height=1000, res=300)

grid_arrange_shared_legend <- function(..., ncol = length(list(...)), nrow = 1, position = c("bottom", "right")) {

  plots <- list(...)
  position <- match.arg(position)
  g <- ggplotGrob(plots[[1]] + theme(legend.position = position))$grobs
  legend <- g[[which(sapply(g, function(x) x$name) == "guide-box")]]
  lheight <- sum(legend$height)
  lwidth <- sum(legend$width)
  gl <- lapply(plots, function(x) x + theme(legend.position="none"))
  gl <- c(gl, ncol = ncol, nrow = nrow)

  combined <- switch(position,
                     "bottom" = arrangeGrob(do.call(arrangeGrob, gl),
                                            legend,
                                            ncol = 1,
                                            heights = unit.c(unit(1, "npc") - lheight, lheight)),
                     "right" = arrangeGrob(do.call(arrangeGrob, gl),
                                           legend,
                                           ncol = 2,
                                           widths = unit.c(unit(1, "npc") - lwidth, lwidth)))

  grid.newpage()
  grid.draw(combined)

  # return gtable invisibly
  invisible(combined)

}

grid_arrange_shared_legend(p1, p2, ncol = 2, nrow = 1, position="right")

dev.off()


