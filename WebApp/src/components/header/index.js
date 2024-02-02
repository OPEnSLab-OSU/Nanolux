import { h } from 'preact';
import { Link } from 'preact-router/match';
import style from './style.css';

const Header = () => (
	<header className={style.header}>
		<a href="https://www.cymaspace.org/" target="_blank" rel="noopener noreferrer" className={style.logo}>
			<img src="../../assets/cymalog1.png" alt="CymaSpace Logo" height="32" />
            <h1 className={style.name}>AudioLux</h1>
		</a>
		<nav>
			<Link activeClassName={style.active} href="/">
				Settings
			</Link>
			{/*<Link activeClassName={style.active} href="/user">*/}
			{/*	User*/}
			{/*</Link>*/}
			<Link activeClassName={style.active} href="/wifi">
				Wifi
			</Link>
		</nav>
	</header>
);

export default Header;
