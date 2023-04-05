import { h } from 'preact';
import { Link } from 'preact-router/match';
import style from './style.css';

const Header = () => (
	<header className={style.header}>
		<a href="/" className={style.logo}>
			<img src="../../assets/Transparent-Cymaspace-Logo-205x71px.png" alt="CymaSpace Logo" height="32" />
			<h1>AudioLux</h1>
		</a>
		<nav>
			<Link activeClassName={style.active} href="/">
				Settings
			</Link>
			<Link activeClassName={style.active} href="/profile">
				User
			</Link>
		</nav>
	</header>
);

export default Header;
