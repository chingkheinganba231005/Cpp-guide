import re,html,sys
for f in sys.argv[1:]:
    s=open(f).read(); print('=====',f)
    for m in re.findall(r'<div class="out[^"]*">(.*?)</div></div>',s,re.S):
        t=html.unescape(re.sub('<[^>]+>','',m)).replace('Predict the output first, then click to check','[predict]')
        print('---',t[:350])
