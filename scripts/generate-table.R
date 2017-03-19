library(xtable)
library(reshape2)

options(xtable.floating = FALSE)
options(xtable.timestamp = "")

outputs="./outputs/tight"
results <- read.csv(paste(outputs,"results",sep="/"),sep="",header=T)


solved <- aggregate(results$found, by=list(Heuristic=results$heuristic,Domain=results$domain), FUN=sum)
oom <- aggregate(results$oom, by=list(Heuristic=results$heuristic,Domain=results$domain), FUN=sum)
oot <- aggregate(results$oot, by=list(Heuristic=results$heuristic,Domain=results$domain), FUN=sum)

summary <- merge(solved, oom, by=c("Heuristic", "Domain"))
summary <- merge(summary, oot, by=c("Heuristic", "Domain"))

names(summary)[names(summary)=="x.x"] <- "Solved"
names(summary)[names(summary)=="x.y"] <- "OOM"
names(summary)[names(summary)=="x"] <- "OOT"

solvedtotals <- aggregate(summary$Solved, by=list(Heuristic=summary$Heuristic), FUN=sum)
oomtotals <- aggregate(summary$OOM, by=list(Heuristic=summary$Heuristic), FUN=sum)
oottotals <- aggregate(summary$OOT, by=list(Heuristic=summary$Heuristic), FUN=sum)

totals <- merge(solvedtotals, oomtotals, by=c("Heuristic"))
totals <- merge(totals, oottotals, by=c("Heuristic"))

names(totals)[names(totals)=="x.x"] <- "Solved"
names(totals)[names(totals)=="x.y"] <- "OOM"
names(totals)[names(totals)=="x"] <- "OOT"

summary$SMT <- paste(summary$Solved, summary$OOM, summary$OOT, sep=" / ")
totals$SMT <- paste(totals$Solved, totals$OOM, totals$OOT, sep=" / ")
totals$Domain <- "Total"

summary <- rbind(summary, totals)

summarytable <- acast(summary, Domain~Heuristic, value.var="SMT")

xtable(summarytable)

#status <- data.frame(S=results$found,
#                     M=results$oom,
#                     T=results$oot)

#results$status <- names(status)[row(t(status))[t(status)==1]]

#resultsftbl <- ftable(results$heuristic, results$domain, results$status,
#                      row.vars = c(2))

#print.xtableFtable(xtableFtable(resultsftbl, method="col.compact"))
