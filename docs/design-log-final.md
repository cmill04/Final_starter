# Final Project Design Log
**Author:** <your name> · **P2 code used:** <my own | instructor reference> · **P3 code used:** <my own | instructor reference>
**Tools used:** <AI assistants / other tools, and what you used them for>

<!-- 700–1,000 words. An argument, not a diary: say what you decided, what
     you rejected, and what evidence backs it up. Paste the numbers your
     tests print. Delete these comments before submitting. -->

## 1. LRU cache: why get and put are O(1)
<!-- List every pointer change in (a) a get() hit on a middle node and
     (b) a put() that evicts. Then argue O(1). -->

## 2. Heap and top_k
<!-- Why is top_k O(n log k)? One measurement against a full sort. -->

## 3. BST height
<!-- Your printed heights for ids 1..1000 inserted in order vs. shuffled.
     Why do sequential ids degenerate the tree, what does it do to find(),
     and what would fix it? -->

## 4. Three colors in DFS
<!-- Why gray and black must be different. Give a small graph where a
     visited/not-visited DFS would report a cycle that is not there. -->

## 5. Inverted index vs. linear scan
<!-- Your timing table at 100 / 1,000 / 5,000 documents, and what the trend
     shows. -->

| documents | index (ms/query) | linear (ms/query) |
|---|---|---|
| 100 | | |
| 1,000 | | |
| 5,000 | | |

## 6. Threat model
<!-- 3–5 sentences: what does the default policy chain defend against, and
     what does it NOT? (Hint: python3 is on the allowlist.) -->

## 7. What I would design differently
