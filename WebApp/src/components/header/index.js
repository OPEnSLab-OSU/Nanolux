import { h } from 'preact';
import { Link } from 'preact-router/match';
import style from './style.css';

const Header = () => (
	<header className={style.header}>
		<a href="/" className={style.logo}>
			<img src="../../assets/cropped-cymaspace-logo-32x32.png" alt="CymaSpace Logo" height="32" width="32" />
			<h1>AudioLux</h1>
		</a>
		<nav>
			<Link activeClassName={style.active} href="/">
				Home
			</Link>
			<Link activeClassName={style.active} href="/profile">
				Me
			</Link>
			<Link activeClassName={style.active} href="/profile/john">
				John
			</Link>
		</nav>
	</header>
);

export default Header;
