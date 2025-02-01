
if(NOT DEFINED DIR_DEPTH)
  set(DIR_DEPTH 0)
else()
  MATH(EXPR DIR_DEPTH "${DIR_DEPTH}+1")
endif()
string(REPEAT "\t" ${DIR_DEPTH} PREFIX_MSG)
MATH(EXPR color_code "${DIR_DEPTH}+33")
MATH(EXPR color_code2 "${DIR_DEPTH}+34")