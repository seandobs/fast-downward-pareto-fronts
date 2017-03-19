library(ggplot2)

outputs="./outputs/loose-stats"

f1 <- paste(outputs, "ff-d", "nodestats", sep="/")
f2 <- paste(outputs, "ipdb-astar", "nodestats", sep="/")
f3 <- paste(outputs, "ipdb-pts", "nodestats", sep="/")

numpoints=512

nodestats <- read.csv(f1,sep=" ",header=T)
nodestats$f <- nodestats$g + nodestats$h
dg = density(x=nodestats$g, n=numpoints)
dh = density(x=nodestats$h, n=numpoints)
df = density(x=nodestats$f, n=numpoints)
du = density(x=nodestats$u, n=numpoints)
baseline <- data.frame(dg.x=dg$x, dg.y=dg$y,
                    dh.x=dh$x, dh.y=dh$y,
                    df.x=df$x, df.y=df$y,
                    du.x=du$x, du.y=du$y,
                    heuristic="FF-d")

gc()

nodestats <- read.csv(f2,sep=" ",header=T)
nodestats$f <- nodestats$g + nodestats$h
dg = density(x=nodestats$g, n=numpoints)
dh = density(x=nodestats$h, n=numpoints)
df = density(x=nodestats$f, n=numpoints)
du = density(x=nodestats$u, n=numpoints)
densities <- rbind(baseline,
                   data.frame(dg.x=dg$x, dg.y=dg$y, dg.z=dg$y - baseline$dg.y,
                        dh.x=dh$x, dh.y=dh$y, dh.z=dh$y - baseline$dh.y,
                        df.x=df$x, df.y=df$y, df.z=df$y - baseline$df.y,
                        du.x=du$x, du.y=du$y, du.z=du$y - baseline$du.y,
                        heuristic="iPDB A*"))
gc()

nodestats <- read.csv(f3,sep=" ",header=T)
nodestats$f <- nodestats$g + nodestats$h
dg = density(x=nodestats$g, n=numpoints)
dh = density(x=nodestats$h, n=numpoints)
df = density(x=nodestats$f, n=numpoints)
du = density(x=nodestats$u, n=numpoints)
densities <-rbind(densities,
                  data.frame(dg.x=dg$x, dg.y=dg$y, dg.z=dg$y - baseline$dg.y,
                             dh.x=dh$x, dh.y=dh$y, dh.z=dh$y - baseline$dh.y,
                             df.x=df$x, df.y=df$y, df.z=df$y - baseline$df.y,
                             du.x=du$x, du.y=du$y, du.z=du$y - baseline$du.y,
                             heuristic="iPDB PTS"))

gc()

png(paste(outputs,"gdensity.png",sep="/"), width = 1920, height=1080, res=220)

ggplot(densities, aes(x = dg.x, y=dg.y, fill = heuristic, color = heuristic, linetype = heuristic)) +
    geom_line() +
    labs(title="g-value Density", x="Normalised g-value", y="Density")

dev.off()

png(paste(outputs,"udensity.png",sep="/"), width = 1920, height=1080, res=220)

ggplot(densities, aes(x = du.x, y=du.y, fill = heuristic, color = heuristic, linetype = heuristic)) +
    geom_line() +
    labs(title="u-value Density", x="u-value", y="Density")

dev.off()

png(paste(outputs,"hdensity.png",sep="/"), width = 1920, height=1080, res=220)

ggplot(densities, aes(x = dh.x, y=dh.y, fill = heuristic, color = heuristic, linetype = heuristic)) +
    geom_line() +
    labs(title="h-value Density", x="Normalised h-value", y="Density")
dev.off()

png(paste(outputs,"fdensity.png",sep="/"), width = 1920, height=1080, res=220)

ggplot(densities, aes(x = df.x, y=df.y, fill = heuristic, color = heuristic, linetype = heuristic)) +
    geom_line() +
    labs(title="f-value Density", x="Normalised f-value", y="Density")
dev.off()
