#!/bin/bash
echo "single ascending"
diff Sorting_Algorithms/SingleAscending.txt CorrectOutput/SingleAscending.txt
echo "single descending"
diff Sorting_Algorithms/SingleDescending.txt CorrectOutput/SingleDescending.txt
echo "single lastLetter"
diff Sorting_Algorithms/SingleLastLetter.txt CorrectOutput/SingleLastLetter.txt
echo "multi ascending"
diff Sorting_Algorithms/MultiAscending.txt CorrectOutput/MultiAscending.txt
echo "multi descending"
diff Sorting_Algorithms/MultiDescending.txt CorrectOutput/MultiDescending.txt
echo "multi lastLetter"
diff Sorting_Algorithms/MultiLastLetter.txt CorrectOutput/MultiLastLetter.txt