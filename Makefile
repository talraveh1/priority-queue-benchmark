# TEST Candidates
# A) std::priority_queue
# B) boost::pairing_heap
# C) pairing_heap_priqueue
# D) skiplist_priqueue
# E) arch-aware heap (int only)

# TEST I: add random elements one by one
# TESTID:
# i) 1k elements
# ii) 10k elements
# iii) 100k elements
# iv) 1M elements
# v) 10M elements
# vi) 100M elements


# TEST II: find the shortest path in graph
# TESTID:
# i) 1k elements
# ii) 10k elements
# iii) 100k elements
# iv) 1M elements
# v) 10M elements
# vi) 100M elements


### note:
# if needed to access local boost library, export BOOST_PATH={PATH_TO_BOOST_LIB}

# Optional: Intel VTune ITT
ITT_ROOT ?= /c/Program Files (x86)/Intel/oneAPI/vtune/latest
ITT_FLAGS ?=
ITT_OBJS :=
ifeq ($(ITT),1)
# prefer the SDK import lib for ittnotify; fall back to top-level if desired
ITT_FLAGS += -DUSE_ITT -I"$(ITT_ROOT)/sdk/include" -I"$(ITT_ROOT)/sdk/src/ittnotify"
ITT_OBJS += ittnotify_static.o
endif

testmain:
	@if (test $(cand) && test $(type) && test $(id)); then \
		if [ "$(ITT)" = "1" ]; then \
			gcc -Ofast -g $(ITT_FLAGS) -c "$(ITT_ROOT)/sdk/src/ittnotify/ittnotify_static.c" -o ittnotify_static.o; \
		fi; \
		g++ -Ofast -g $(EXTRA_FLAGS) $(ITT_FLAGS) $(ITT_OBJS) testmain.cc -o testmain -DTCAND=$(cand) -DTCAND_$(cand) -DTTYPE=$(type) -DTTYPE_$(type) -DTID=$(id) -I $(BOOST_PATH) -DTID_$(id); \
	else \
		echo "arg cand/type/id is missing"; \
		exit 1; \
	fi

run : testmain
	./testmain

clean:
	@[ -f testmain ] && rm testmain || true
