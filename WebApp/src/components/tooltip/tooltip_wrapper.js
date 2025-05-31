import { Tooltip } from 'react-tooltip';
import 'react-tooltip/dist/react-tooltip.css';

/**
 * Props:
 * id: string
 * content: ReactNode
 * offset?: integer (default 10)
 * style?: React.CSSProperties (default { fontSize: '1.2rem', cursor: 'pointer' })
 * label?: React.ReactNode (default '(?)')
 * ...props on the trigger span
 */
const TooltipWrapper = ({
    id,
    content,
    offset = 10,
    style = { fontSize: '1.2rem', cursor: 'pointer' },
    label = '(?)',
    ...props
}) => (
    <>
        <span
          style={style}
          data-tooltip-id={id}
          data-tooltip-offset={offset}
          role='button'
          tabIndex={0}
          aria-describedby={id}
          {...props}
        >
            {label}
        </span>
        <Tooltip
          id={id}
          content={content}
          style={style}
          aria-hidden='true'
        />
    </>
);

export default TooltipWrapper;