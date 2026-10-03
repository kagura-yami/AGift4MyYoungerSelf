const menu = document.querySelector('.menu-toggle');
const navigation = document.querySelector('#navigation');
menu.addEventListener('click', () => { const open = menu.getAttribute('aria-expanded') !== 'true'; menu.setAttribute('aria-expanded', String(open)); navigation.classList.toggle('open', open); });
navigation.addEventListener('click', e => { if (e.target.closest('a')) { menu.setAttribute('aria-expanded', 'false'); navigation.classList.remove('open'); } });
document.addEventListener('keydown',e=>{if(e.key==='Escape' && navigation.classList.contains('open')){navigation.classList.remove('open');menu.setAttribute('aria-expanded','false');menu.focus();}});
const skills = [
 ['平面位移','移动 · DASH','沿移动方向快速冲刺，把不够远的一跳接成一条新的路。','按左 Shift 施放。空中使用后，需要落地才能恢复空中次数；冷却结束不代表可以在空中再次冲刺。'],
 ['二段跳','移动 · UPDASH','在空中再次跃起，带着前进的惯性越过障碍。','按左 Ctrl 施放。不会清除水平速度，起跳后自然减速上升、再落下。每次腾空的可用次数在落地后恢复。'],
 ['蹬墙跳','移动 · WALL JUMP','借墙面再跳一次，让看似尽头的地方变成起点。','在空中面向可蹬跳墙面，按 Space 向外、向上弹跳。不是所有墙面都可以蹬跳，同一墙体附近连续使用也有限制。'],
 ['垫脚石','造物 · STEP STONE','没有落脚点？为自己造一块限时的平台。','按住 Q 预览，松开 Q 放置。留意禁建区、障碍和数量上限；平台存在时间有限，放好之后及时起跳。'],
 ['局部减慢','时间 · SLOW','让移动的平台慢下来，为下一次起跳争取时机。','面向可受时间影响的移动平台，按 F 施放。需要目标在前方有效范围内且没有遮挡；并非对所有物体都有效。'],
 ['时间回溯','时间 · REWIND','沿走过的轨迹退回去，把刚才的失误变成新的机会。','R 回溯自身，T 回溯目标平台，两种操作共享冷却。需要足够的历史记录；自身回溯会清除分身，不会回滚整个世界。'],
 ['分身','空间 · ECHO','把另一个自己留在这里，再去探索另一边。','首次按 C 创建并操控分身，再按 C 在本体与分身间切换。两者保留各自的位置；分身是否能触发机关取决于机关规则。'],
 ['奖励加时','被动 · BONUS TIME','在倒计时开始时，多给自己一点余地。','挑战开始时自动增加奖励判定的时间预算，无需按键。它给你更多争取自选奖励的机会，不会代替你完成挑战。']
];
const tabs = [...document.querySelectorAll('[data-skill]')];
function selectSkill(index,focus=false){
 tabs.forEach((tab,i)=>{tab.setAttribute('aria-selected',String(i===index));tab.tabIndex=i===index?0:-1;});
 const [name,category,description,tip]=skills[index];
 document.querySelector('#skill-name').textContent=name;
 document.querySelector('#skill-category').textContent=`0${index+1} / ${category}`;
 document.querySelector('#skill-description').textContent=description;
 document.querySelector('#skill-tip').textContent=tip;
 document.querySelector('#skill-panel').setAttribute('aria-labelledby',`tab-${index}`);
 document.querySelector('#skill-icon').style.backgroundPosition=`${(index%4)*100/3}% ${index<4?0:100}%`;
 if(focus)tabs[index].focus();
}
tabs.forEach((tab,i)=>{tab.addEventListener('click',()=>selectSkill(i));tab.addEventListener('keydown',e=>{let next=i;if(e.key==='ArrowDown'||e.key==='ArrowRight')next=(i+1)%8;else if(e.key==='ArrowUp'||e.key==='ArrowLeft')next=(i+7)%8;else if(e.key==='Home')next=0;else if(e.key==='End')next=7;else return;e.preventDefault();selectSkill(next,true);});});
if('IntersectionObserver' in window && !matchMedia('(prefers-reduced-motion: reduce)').matches){document.documentElement.classList.add('js-motion');const observer=new IntersectionObserver(entries=>entries.forEach(entry=>{if(entry.isIntersecting){entry.target.classList.add('visible');observer.unobserve(entry.target);}}),{threshold:.06});document.querySelectorAll('.reveal').forEach(el=>observer.observe(el));}
const config=window.GIFT_SITE||{};
if(config.downloadUrl){try{const url=new URL(config.downloadUrl);if(url.protocol==='https:'){const link=document.querySelector('#download-link');link.href=url.href;link.hidden=false;link.rel='noopener';document.querySelector('#download-pending').hidden=true;document.querySelector('#release-status').textContent=config.releaseLabel||'Windows 试玩版';document.querySelector('.release-note').textContent='下载完成后，请完整解压文件夹再启动游戏。';}}catch{ /* 无效地址保持待开放，不生成失效链接。 */ }}
